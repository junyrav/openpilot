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

## 수정 이력

- **2026-09-29 (실차 qlog 반영)**: 오픈파일럿 비활성 상태에서 계기판에 "차로 안전 / 전방·측방 안전 / 차로 유지 보조 시스템 점검"이 뜨던 문제 수정.
  원인: 비활성 0xCB(ADAS_CMD_35)에 STEERING_SENSORS 조향각을 넣었는데, panda는 MDPS 조향각 기준 ±0.1°만 허용 →
  0xCB가 전부 차단되고(패널 txBlocked 12,000회) 순정 ADRV의 0xCB도 막혀 있어 MDPS가 0xCB를 전혀 받지 못함.
  이제 MDPS 조향각(0xEA STEERING_ANGLE)을 사용. 로그 조건 재현 시뮬레이션에서 비활성 0xCB 차단 100% → 0%.

- **2026-09-29 (실차 rlog 반영, panda 펌웨어 변경)**: 위 수정 후에도 경고가 남아 rlog로 비교한 결과, 오픈파일럿이 대신 보내는
  0x12A(LFA)가 순정과 100% 다름(순정 0x12A는 조향각 −65°~175° 동안 3가지 값뿐인 상태 프레임). 이제 오픈파일럿이 제어하지 않을 때는
  순정 ADRV의 0x12A·0xCB를 그대로 통과시키고, 제어 중일 때만 오픈파일럿 프레임으로 교체 (StarPilot의 LKAS_ALT 순정 통과 방식과 동일).
  롱컨 사용 시 0x1A0·0x160·0x1E0은 여전히 오픈파일럿 값으로 교체되며 순정과 다름 → "전방/측방 안전" 경고는 롱컨을 끄면 사라질 것으로 예상.

- **2026-09-29 (자동주행 활성화 안 됨) → 2026-09-30 되돌림**: 0x1CF를 더미로 판단해 0x1AA에서 버튼을 읽도록 바꿨으나 **잘못된 분석**이었음.
  0x1CF는 카운터가 정상 순환하는 살아있는 메시지이고(15가지 값 순환), 해당 로그 구간에 버튼 입력이 없었을 뿐. 0x1AA의 byte6~8은
  계기판 속도(CLU_SPEED). CarrotPilot도 이 차에서 0x1CF를 읽음 → 원래대로 0x1CF 사용.

- **2026-09-29 (조향 안 됨, panda 펌웨어 변경)**: 이전 rlog에서 순정 HDA2가 실제로 조향한 약 8초 구간을 찾아 비교.
  순정은 조향 중 0x12A byte3=0x18, 그 외 바이트(토크 raw 0, byte9-12 차선 정보)는 그대로. 오픈파일럿은 기본값으로 만든
  0x12A를 보내고 있었음 → 활성 중에는 최신 순정 0x12A를 그대로 복사하고 byte3만 순정 조향 패턴(0x18)으로 바꿔 보냄.
  panda 규칙도 이 형태(차선 정보 포함, 토크 raw 0)를 상태 프레임으로 인정하도록 수정 (조향 요청 비트·토크는 계속 차단).

- **2026-09-30 (롱컨 시 계기판 경고·HUD 미연동, 당근파일럿 방식 적용)**: 41d1ef8 실주행 rlog에서 조향(명령 대비 실제 조향각 중앙값 오차 0.3°)과
  롱컨 동작 확인. 남은 문제는 롱컨 시 초반 경고 3개와 계기판 HUD 미연동. 원인: StarPilot이 camera-SCC 롱컨에서 0x160에 "AEB 꺼짐" 표시를
  일부러 보내고, 0x1E0/0x1A0은 기본값으로 만들어 보냄. 당근파일럿 코드를 확인해 같은 방식 적용:
  ccNC(CCNC) 경로 사용 → 0x161/0x162 계기판 HUD를 오픈파일럿이 보내고 0x160은 순정 그대로 통과,
  0x1E0(LFAHDA_CLUSTER)은 순정 복사 후 HDA/LFA 상태만 변경, 0x1A0(SCC_CONTROL)은 순정 복사 후 제어 필드만 오픈파일럿 값으로,
  가속을 막는 필드(SysFailState·TakeOverReq·AccelLimitBand 등)는 0. 실제 rlog 3개 세그먼트 재생에서 활성 중 panda 차단 0건.

- **2026-09-30 (계기판에 차량이 범퍼 바로 앞에 표시됨)**: ccNC HUD가 크루즈 메인만 켜져 있으면 선행차를 항상 표시하고, 거리는 ADRV 버스의
  카메라 선행차 메시지(0x1B5)에서 가져오는데 이 차는 0x1B5가 E-CAN(bus 0)에만 있어 거리 0 → 범퍼 앞 차량. 이 차는 오픈파일럿 레이더/모델의
  선행차로 표시(없으면 순정처럼 LEAD 0 / 204.6 m). 실제 rlog 재생: 선행차 있을 때만 116 m·32 m 등 실제 거리로 표시.
  남은 경고 2개는 순정 ADRV가 스스로 FAULT_LFA/FAULT_DAS를 띄우는 것으로 확인(이전 버전 로그에도 존재) — LKAS 버튼이나 SET으로
  순정 LFA/HDA가 같이 켜졌는데 조향 명령이 막히면서 발생하는 것으로 보임.

- **2026-09-30 (전방/측방 안전·차로 변경 보조 점검 경고, panda 펌웨어 변경)**: e11fa5e부터 순정 ADRV가 시동 약 6.5초 후
  FAULT_FCA·FAULT_LCA·FAULT_DAS를 스스로 띄움(41d1ef8 로그에는 없음). 원인: ccNC TX 목록이 bus 2의 0xEA(MDPS)·0x7C4를
  "오픈파일럿이 대신 보내는 프레임"으로 예약해서 실제 MDPS 프레임이 ADRV로 전달되지 않음(로그: bus0→ADRV 미전달 메시지 0xEA 하나).
  이 차 경로(LFA 각도조향)는 대신 보내지 않으므로 0xEA/0x7C4를 다시 ADRV로 전달. 안전 테스트 1945건 통과.

- **2026-10-01 (경고 2개·LKAS 조향 에러·크루즈 표시, 당근파일럿 인터페이스 매칭, panda 펌웨어 변경)**:
  - 원인: 순정 ADRV는 자신의 0xCB 조향 상태(byte3 bit4-5)와 MDPS 0xEA의 각도제어 상태(byte18 bit0-1)가 일치하는지 감시함.
    오픈파일럿이 조향하면 MDPS는 오픈파일럿을 따르므로 ADRV 입장에서 모순 → FAULT_LFA/LCA/FCA 래치
    (aeeeec3 로그: 불일치 2,297프레임, 41d1ef8 로그: 조향 해제 0.5초 뒤 FAULT_LFA). LKAS 버튼 → 순정 LFA가 조향 시도 → 같은 모순.
  - 수정: 당근파일럿처럼 ADRV로 전달되는 MDPS 프레임의 그 2비트만 ADRV 자신의 값으로 바꿔 전달(순정 프레임 슬롯·카운터 그대로, 체크섬만 재계산).
    오픈파일럿이 조향권을 갖지 않을 때(순정 통과 상태)는 실제 MDPS 그대로. 실제 rlog 재생: ADRV가 보는 불일치 2,297 → 7프레임(전환 순간).
  - 크루즈 메인: 순정/당근처럼 시동 시 꺼짐, 크루즈 메인 버튼으로 켬/끔. 메인이 꺼져 있으면 계기판 크루즈 아이콘 없음(SETSPEED 0).
  - 계기판으로 보내는 0x162에서 FAULT_LCA 숨김(당근과 동일). **FAULT_FCA(전방충돌방지)는 숨기지 않음** — 계속 뜨면 실제 문제.
  - 운전자 조향 토크/터치 위조(당근의 STEER_TOUCH, +220 토크 주입)는 운전자 감시 우회라 적용하지 않음.

- **2026-10-01 (설정 속도보다 3~4 km/h 빠름, LKAS→AOL, 핸들 터치, panda 펌웨어 변경)**:
  - 계기판 속도는 실제 바퀴 속도보다 약 2~3 km/h 높게 표시됨(로그: 실제 70.7 → 계기판 73.5). 오픈파일럿은 실제 속도를 설정 속도에 맞춰서
    계기판에는 더 빠르게 보였음. 0x1AA byte6(0.5 km/h 단위, 당근의 CLU_SPEED와 동일)을 vEgoCluster로 사용 → 순정 SCC처럼 계기판 속도 = 설정 속도.
  - 첫 부팅 1회: Always On Lateral ON + LKAS 버튼 = AOL 토글(LKASButtonControl 9). AOL은 주행 중(시동 ON) 변경 불가라 부팅 시 적용.
  - 당근처럼 ADRV가 보는 MDPS 사본에만 10초마다 0.4초간 핸들 토크 +220 추가(오픈파일럿이 조향권을 가질 때만). 오픈파일럿 운전자 모니터링은 그대로.
    핸들 터치 센서(0x2AF)는 체크섬 방식이 확인되지 않아 건드리지 않음.

- **2026-10-02 (LKAS 대기 핸들 표시)**: Always On Lateral이 켜져 있지만 조향 중이 아니면 계기판에 대기(흰/회색) 핸들,
  조향 중이면 활성(초록) 핸들 표시 — 당근/순정과 동일. 0x161 LFA_ICON 1/2.

## 당근파일럿 vs StarPilot 차량 인터페이스 비교 (이 차 경로: camera SCC + LFA 각도조향)

| 메시지 | 당근파일럿 | StarPilot K8-HEV-PE (현재) |
|---|---|---|
| 0xCB 조향각 → MDPS | 순정 복사 + 각도/활성 덮어씀, panda가 순정 프레임 슬롯에 교체(순정 카운터) | 오픈파일럿 생성(자체 카운터), 제어 중에만 교체, 해제 시 순정 통과. panda 각도 안전검사 유지 |
| 0x12A LFA | 순정 복사 + 상태 필드 | 순정 복사 + byte3만 변경(제어 중), 해제 시 순정 통과 |
| 0x1A0 SCC_CONTROL | 순정 복사 + 제어 필드, 슬롯 교체 | 순정 복사 + 제어 필드(자체 카운터) |
| 0x1E0 LFAHDA | 순정 복사 + HDA/LFA 심볼 | 동일 |
| 0x161/0x162 계기판 | 순정 복사 + HUD 필드, FAULT_LCA/HDA/DAS 숨김 | 순정 복사 + HUD 필드, FAULT_LCA/HDA/DAS/LFA/LSS/DAW/ESS 숨김, FCA 표시 |
| 0x160 | 순정 통과 | 순정 통과 |
| **0xEA MDPS → ADRV** | 순정 복사 + LFA2_ACTIVE=ADRV 자신의 값, 주기적 토크 +220 | ADRV 자신의 값 에코 + 주기적 토크 +220 (순정 슬롯·카운터 유지) |
| 0x175 TCS → ADRV | DriverBraking=0 등 수정해서 교체 | 순정 그대로 |
| 0x2AF 핸들 터치 → ADRV | 10초마다 터치 위조 | 순정 그대로 (체크섬 미확인) |
| 0x1CF 버튼 → ADRV | 순정 + 메인/LFA/RES 버튼 주입(순정 SCC·LFA를 켜둠) | 순정 그대로 |
| 계기판 속도(0x1AA) | vEgoCluster로 사용 | vEgoCluster로 사용 (이번 수정) |
| panda 조향 안전검사 | 주석 처리(꺼짐) | 유지 |

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
- 크루즈는 시동 후 꺼져 있습니다. 크루즈 메인 버튼을 먼저 누른 뒤 SET/RES로 활성화하세요(Always On Lateral도 메인이 켜져야 동작).
