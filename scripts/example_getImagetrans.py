import argparse
import platform

import cv2


class Camera:
    def __init__(self, host: str, port: int) -> None:
        self._gst_pipeline = self._build_gstreamer_cmd(host, port)
        self.frame = None

        print(f"GStreamer from {host}:{port}")

    def _get_cpu_arch_decoder(self) -> str:
        if platform.machine() == "x86_64":
            return "avdec_h264"
        else:
            return "omxh264dec"

    def _build_gstreamer_cmd(self, host: str, port: int) -> str:
        str_address = f"udpsrc address={host} port={port} ! "
        str_application = "application/x-rtp,media=video,encoding-name=H264 ! "
        str_decoder = f"rtph264depay ! h264parse ! {self._get_cpu_arch_decoder()} ! videoconvert ! appsink"

        return str_address + str_application + str_decoder

    def get_img(self):
        cap = cv2.VideoCapture(self._gst_pipeline)

        while True:
            ret, frame = cap.read()

            if not ret:
                print("Error: Could not read frame.")
                break

            if frame is not None:
                self.frame = frame.copy()

            cv2.imshow("frame", frame)

            if cv2.waitKey(2) & 0xFF == ord("q"):
                break

        cap.release()
        cv2.destroyAllWindows()


if __name__ == "__main__":

    parser = argparse.ArgumentParser(
        description="GStreamer OpenCV Video Capture"
    )
    parser.add_argument(
        "--address",
        type=str,
        default="192.168.123.200",
        required=True,
        help="UDP address, which is the IP of this PC: [192.168.123.xx]",
    )
    parser.add_argument(
        "--port",
        type=int,
        default="9201",
        required=True,
        help="UDP port: [9201, 9202, 9203, 9204, 9205]",
    )

    args = parser.parse_args()

    camera = Camera(
        host=args.address,
        port=args.port,
    )
    camera.get_img()
