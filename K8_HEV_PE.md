# StarPilot K8-HEV-PE 브랜치

2026 기아 K8 하이브리드 페이스리프트(GL3 PE, HDA2/LFA2, CAN-FD) + Comma 4 + ADAS 18핀 하네스 전용 브랜치입니다.
베이스: `firestar5683/StarPilot` `StarPilot` 브랜치 (2a12dbd, 2026-09-22, prebuilt)

## 설치

Comma 4 설정 화면에서 **Custom Software** 선택 후 입력:

```
installer.comma.ai/<GitHub아이디>/K8-HEV-PE
```

첫 부팅 시 패널(panda) 펌웨어가 자동으로 다시 플래시됩니다(수십 초, 정상 동작).

## 첫 부팅 시 자동 설정 (1회만)

| 항목 | 값 |
|---|---|
| 차량 선택 (수동) | Kia K8 Hybrid (with HDA II) 2023 (`KIA_K8_HEV_1ST_GEN`) |
| Disable Fingerprinting | ON (2026 PE 펌웨어 버전이 DB에 없으므로 수동 선택 고정) |
| openpilot 롱컨 (Alpha Longitudinal) | ON |

이후 UI에서 바꾼 값은 유지됩니다. 다시 적용하려면 SSH로 `rm /data/k8_pe_preset_v1` 후 재부팅.

## 차량 CAN 구성 (CarrotPilot 로그 기반, 자동 감지로 동일하게 설정됨)

| 항목 | 값 |
|---|---|
| E-CAN / A-CAN / ADRV(CAM) | bus 0 / 1 / 2 |
| CAN-FD | 500 kbps / 2 Mbps |
| 조향 | 각도 기반, `ADAS_CMD_35_10ms` 0xCB (24 B, 100 Hz) + LFA 0x12A 상태 프레임 |
| SCC | Camera SCC, `SCC_CONTROL` 0x1A0 (32 B, 50 Hz) |
| A-CAN LKAS 상태 | `LKAS_ALT` 0x110 (32 B, 100 Hz), bus 1 |
| 하이브리드 | 0xFA 로 자동 감지 |

감지 결과 플래그: `CANFD | CANFD_ANGLE_STEERING | CANFD_CAMERA_SCC | SEND_LFA | HYBRID`,
panda safety: `hyundaiCanfd` + `CANFD_ANGLE_STEERING | CAMERA_SCC | HYBRID_GAS | LONG`.

## 변경 사항

1. `opendbc/car/hyundai/values.py` — `KIA_K8_HEV_1ST_GEN`에 `CANFD_ANGLE_STEERING` 적용 (각도조향).
2. `opendbc/car/hyundai/interface.py` — 0x110이 bus 2에서 보이면(하네스 위치 다름) 로그에 경고 기록.
3. `opendbc/safety/modes/hyundai_canfd.h` (panda 펌웨어) — **LFA 조향 + 각도조향 버그 수정**:
   원본에서는 0xCB와 함께 보내는 0x12A 상태 프레임까지 각도 명령으로 검사해서
   (a) 실시간 각도 변화율 한도(250 ms당 31프레임)를 2배로 소모 → 조향 프레임의 약 36%가 차단되고,
   (b) 오픈파일럿 해제 상태에서 0x12A가 전부 차단되었습니다.
   이제 조향 요청·각도·게인·토크가 모두 0인 0x12A 상태 프레임은 통과시키고, 0x12A에 토크가 실리면 항상 차단합니다.
   0xCB 각도 명령의 검사는 그대로 유지됩니다.
4. `panda/board/obj/*` — 위 수정이 반영된 panda 펌웨어 재빌드 (StarPilot 원본과 동일한 debug 키로 서명).
5. `starpilot/system/k8_pe_defaults.py`, `system/manager/manager.py` — 첫 부팅 차량 프리셋.
6. `opendbc/safety/tests/test_hyundai_canfd.py` — 위 safety 동작 테스트 4건 추가.

## 검증 (PC 시뮬레이션)

- 로그 버스 구성으로 만든 fingerprint → 위 플래그/버스(0/1/2) 확인.
- 차량 컨트롤러 400프레임(4초) 출력 전체를 실제 panda safety 코드에 통과: 0xCB·0x12A·0x1A0·0x160·0x1E0 모두 허용, 차단 0건.
  (원본 safety에서는 0xCB/0x12A 각 144프레임 차단)
- 0xCB 20° 급변 명령·제어 비허용 상태 조향 명령은 여전히 차단됨.
- Hyundai safety 테스트 1943건 통과, Hyundai car 테스트는 원본과 동일한 결과.

## 주의

- **실차 미검증입니다.** 첫 주행은 빈 도로에서 저속으로, 언제든 개입할 준비를 하고 확인하세요.
- openpilot 롱컨 사용 중에는 순정 SCC가 차단되므로 **순정 전방충돌방지(FCA/AEB) 기능이 동작하지 않을 수 있습니다.**
- 페이스리프트 이전(토크 조향) K8에는 이 브랜치를 사용하지 마세요.
- ccNC 계기판 연동(`CCNC` 플래그)은 로그로 확인되지 않아 켜지 않았습니다.
