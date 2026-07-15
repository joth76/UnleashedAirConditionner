Import("env", "projenv")

import os
import subprocess

def sign_firmware(source, target, env):
    project_dir = env['PROJECT_DIR']
    firmware_path = target[0].rstr()
    signed_firmware_path = firmware_path.replace(".bin", ".signed.bin")

    upload_port = env.GetProjectOption("upload_port")
    upload_flags = env.GetProjectOption("upload_flags") or []
    auth_flag = next((flag for flag in upload_flags if "--auth" in flag), None)

    if auth_flag:
        password = auth_flag.split("=", 1)[1] if "=" in auth_flag else None
    else:
        password = None

    # Path to the private key
    private_key_path = os.path.join(project_dir, "signing", "private.key")

    # Check if keys folder and private.key exist
    if not os.path.exists(private_key_path):
        print("Private key not found. Skipping signing process.")
        return

    # Command to sign the firmware
    signing_command = f"python ~/.platformio/packages/framework-arduinoespressif8266/tools/signing.py --mode sign --privatekey {private_key_path} --bin {firmware_path} --out {signed_firmware_path}"
    print("Signing Command:", signing_command)

    print("Signing firmware...")
    subprocess.run(signing_command, shell=True)

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", sign_firmware)
