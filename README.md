# Macropads - Custom Mechanical Macropad

![GitHub license](https://img.shields.io/github/license/inu-alik/Macropads)
![Hardware](https://img.shields.io/badge/Hardware-KiCad-blue)
![Firmware](https://img.shields.io/badge/Firmware-Arduino-orange)

Macropads adalah sebuah perangkat *custom mechanical macropad* yang dirancang untuk meningkatkan produktivitas, alur kerja *editing* (video/foto), serta pemrograman. Proyek ini mencakup seluruh proses pengembangan, mulai dari perancangan skematik dan layout PCB, pemilihan komponen, hingga penulisan *firmware* berbasis Arduino IDE.

Perangkat ini dilengkapi dengan tombol mekanis yang dapat diprogram (*fully programmable keys*) dan Rotary Encoder untuk kendali yang lebih dinamis.

---

## 🚀 Fitur Utama

* **Fully Programmable Keys:** Mendukung *custom keymapping* dan *multi-layer switching* untuk berbagai software (VS Code, Premiere Pro, After Effects, dll).
* **Rotary Encoder Integration:** Digunakan untuk kendali presisi seperti *volume control*, *scrolling*, atau *timeline scrubbing* pada aplikasi editing.
* **Hot-swappable Switch Sockets:** Menggunakan socket Kailh/Outemu agar pengguna dapat mengganti *mechanical switch* tanpa perlu menyolder ulang.
* **OLED Display (128x32):** Menampilkan status layer yang aktif, mode makro, atau animasi kustom.

---

## 🛠️ Spesifikasi Teknis

### Hardware
* **Microcontroller:**  ESP32
* **PCB Design Software:** KiCad
* **Connectivity:** USB Type-C
* **Switch Compatibility:** Cherry MX Style 5-pin

### Firmware & Software
* **Firmware Base:** Custom C++ via Arduino IDE
* **Communication Protocol:** USB HID (Human Interface Device)

---

## 📐 Desain Perangkat Keras (Hardware)

Proses perancangan PCB dilakukan sepenuhnya menggunakan **KiCad**. Desain berfokus pada efisiensi jalur (*routing*), penempatan komponen yang ergonomis, serta kemudahan saat proses perakitan (*soldering*).

### Fitur Hardware:
* **Microcontroller:** Menggunakan ESP32 / Arduino (Pro Micro) sebagai otak utama dengan komunikasi USB HID.
* **Key Matrix:** Menggunakan konfigurasi matriks dioda untuk menghemat pin I/O (atau direct pin jika menggunakan mikrokontroler dengan banyak pin).
* **Hot-swappable:** Jalur dirancang menggunakan socket khusus agar switch bisa diganti tanpa solder ulang.

### Skematik & Layout PCB
| 3D Render PCB | Top Layer Layout |
| --- | --- |
| ![3D Render](assets/render3d.png) | ![Top Layer](assets/layout.png) |

*Catatan: File gerber siap cetak dan daftar komponen lengkap tersedia di folder `/Hardware`.*

---

## 💻 Pengembangan Firmware

Macropad ini menggunakan matriks pemindaian tombol (*key matrix scanning*) [atau pembacaan langsung pin GPIO jika jumlah pin cukup] untuk mendeteksi input dengan latensi rendah.

Contoh konfigurasi pemetaan tombol (Keymap Layer 0 - Default):
```text
┌─────┐┌─────┐┌─────┐
│ MUTE││ PLAY││ NEXT│  <-- Rotary Encoder (Click/Left/Right)
└─────┘└─────┘└─────┘
┌─────┐┌─────┐┌─────┐
│ ESC ││  /  ││  *  │
├─────┼─────┼─────┤
│  7  ││  8  ││  9  │
├─────┼─────┼─────┤
│ CTRL││  Z  ││  Y  │  <-- Shortcut Undo/Redo
└─────┘└─────┘└─────┘
