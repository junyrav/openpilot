"""
K8-HEV-PE branch: one-time vehicle preset for the 2026 Kia K8 Hybrid facelift (GL3 PE, HDA II / LFA2).

On the first boot of this branch it:
  * selects "Kia K8 Hybrid (with HDA II) 2023" (KIA_K8_HEV_1ST_GEN) manually and disables fingerprinting,
    because the 2026 PE firmware versions are not in the fingerprint database yet;
  * turns on openpilot longitudinal control (alpha long).

It runs only once (marker file), so anything the driver changes afterwards in the UI is kept.
Delete the marker file to re-apply the preset.
"""
import os

from openpilot.common.params import Params
from openpilot.common.swaglog import cloudlog

K8_PE_CAR_MODEL = "KIA_K8_HEV_1ST_GEN"
K8_PE_CAR_MODEL_NAME = "Kia K8 Hybrid (with HDA II) 2023"
K8_PE_CAR_MAKE = "Kia"

MARKER_PATH = os.getenv("K8_PE_PRESET_MARKER", "/data/k8_pe_preset_v1")


def apply_k8_pe_defaults(params: Params, marker_path: str = MARKER_PATH) -> bool:
  if os.path.exists(marker_path):
    return False

  try:
    params.put("CarMake", K8_PE_CAR_MAKE)
    params.put("CarModel", K8_PE_CAR_MODEL)
    params.put("CarModelName", K8_PE_CAR_MODEL_NAME)
    params.put_bool("ForceFingerprint", True)

    params.put_bool("AlphaLongitudinalEnabled", True)
    params.put_bool("DisableOpenpilotLongitudinal", False)
    # force CarParams to be rebuilt with the new selection
    params.remove("CarParamsCache")
  except Exception:
    cloudlog.exception("k8_pe_defaults: failed to apply preset")
    return False

  try:
    os.makedirs(os.path.dirname(marker_path), exist_ok=True)
    with open(marker_path, "w") as f:
      f.write("applied\n")
  except OSError:
    cloudlog.exception("k8_pe_defaults: failed to write marker")

  cloudlog.warning("k8_pe_defaults: applied 2026 K8 HEV PE preset")
  return True
