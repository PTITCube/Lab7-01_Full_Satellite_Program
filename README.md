# Lab 7-01: Tích Hợp Toàn Bộ Phần Mềm Vệ Tinh Tuần Tự (Full Satellite Program)

> **Phân hệ:** Integration & Flight Software (FSW)  
> **Thư viện yêu cầu:** `PTITCube Library v1.1.0`  
> **Nền tảng:** PlatformIO / ESP32-WROOM-32 (OBC)

---

## 📖 1. Mục Tiêu & Mô Tả Bài Thực Hành

Tích hợp toàn diện 4 phân hệ chính (EPS, C&DH, Sensor, COM) theo mô hình Máy trạng thái hữu hạn (FSM) trong vòng lặp tuần tự (Sequential Loop).

---

## 🔌 2. Sơ Đồ Chân Phần Cứng (Hardware Pinout)

| Phân hệ | Giao thức | Chân GPIO |
| :--- | :--- | :--- |
| EPS (MAX17048) | I2C | GPIO 21 (SDA), GPIO 22 (SCL) |
| C&DH (MicroSD) | SPI | CS=13, SCK=18, MISO=19, MOSI=23 |
| Sensor (BMP280/MPU6500) | I2C | GPIO 21, GPIO 22 |
| Sensor (GNSS L76X) | UART2 | RX=16, TX=17 |
| COM (LoRa E32) | UART2 / LoRa | 433 MHz |

---

## 🛠️ 3. Cấu Hình Phần Mềm & Thư Viện

- **Thư viện cốt lõi:** Thư mục `lib/PTITCube` tích hợp sẵn bộ thư viện phiên bản mới nhất `v1.1.0` với các phân hệ phân lớp:
  - `CDH`: Quản lý luồng xuất nhập dữ liệu `cdh` (Stream inheritance) và hệ thống tệp FAT32 trên thẻ nhớ MicroSD.
  - `EPS`: Quản lý năng lượng, đọc IC MAX17048 qua I2C.
  - `Sensor`: Đọc cụm cảm biến môi trường BMP280, cảm biến quán tính 6-DoF MPU6500, từ kế QMC5883L và định vị GNSS L76X.
  - `COM`: Quản lý module vô tuyến LoRa Ebyte E32 qua giao tiếp UART2.
- **Kênh giám sát Console:**
  - **Công cụ:** `cdh (PTIT_CDH stream - 115200 baud)`
  - **Tốc độ Baud:** `115200 bps`
  - **Lưu ý chuẩn hóa:** Kể từ Lab 3, mọi luồng in debug console được chuẩn hóa qua đối tượng `cdh` (`PTIT_CDH cdh;`) kế thừa trực tiếp từ lớp `Stream`/`Print` của Arduino, thay thế hoàn toàn cho `Serial` thông thường.

---

## 🚀 4. Hướng Dẫn Biên Dịch & Nạp Firmware

Sử dụng **PlatformIO CLI** hoặc giao diện **VSCode / PlatformIO IDE**:

```bash
# 1. Di chuyển vào thư mục dự án
cd Lab7-01_Full_Satellite_Program

# 2. Biên dịch mã nguồn
pio run

# 3. Nạp firmware vào vi điều khiển vệ tinh
pio run -t upload

# 4. Mở cửa sổ giám sát Serial Monitor
pio device monitor -b 115200
```

---

## 📝 5. Lưu Ý Kỹ Thuật

- Khởi tạo đối tượng cdh, eps, sensor, com. Điều phối trạng thái BOOT -> SAFE -> MISSION -> CRITICAL.
- Không chèn các lệnh in `println()` trang trí dư thừa làm nghẽn bộ đệm UART khi vệ tinh vận hành ở tần số cao.
- Kiểm tra kết nối cáp USB-UART và nguồn pin trước khi thực hiện nạp code.
