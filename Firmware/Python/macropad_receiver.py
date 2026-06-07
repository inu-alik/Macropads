import serial
import pyautogui
import time
import asyncio
import screen_brightness_control as sbc
from pycaw.pycaw import AudioUtilities
from winsdk.windows.media.control import GlobalSystemMediaTransportControlsSessionManager as MediaManager

# --- Konfigurasi Port Serial ---
SERIAL_PORT = 'COM3'  # Sesuaikan dengan port COM ESP32 Anda di Arduino IDE
BAUD_RATE = 115200

# --- Fungsi Async untuk Mengambil Judul Musik dari Windows ---
async def get_windows_media_info():
    try:
        manager = await MediaManager.request_async()
        session = manager.get_current_session()
        if session:
            props = await session.try_get_media_properties_async()
            if props:
                return f"{props.title} - {props.artist}"
    except:
        pass
    return "Tidak Ada Musik"

# --- Inisialisasi Kontrol Audio Windows ---
try:
    device = AudioUtilities.GetSpeakers()
    volume_control = device.EndpointVolume
    print(f"📦 [SISTEM] Audio terinisialisasi: {device.FriendlyName}")
except Exception as e:
    print(f"❌ [EROR] Gagal menginisialisasi audio: {e}")
    exit()

# --- Inisialisasi Koneksi ESP32 ---
try:
    esp32 = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.05)
    print(f"🔌 [SISTEM] Berhasil terhubung ke MACROPADS di {SERIAL_PORT}")
    print("==========================================================================")
    print("           MONITOR VERIFIKASI AKSI SHORTCUT MACROPADS AKTIF               ")
    print("==========================================================================")
except Exception as e:
    print(f"❌ [EROR] Gagal membuka port {SERIAL_PORT}. Er: {e}")
    exit()

last_music = ""
last_media_check = 0

while True:
    try:
        current_time = time.time()
        
        # Sinkronisasi data musik ke ESP32 setiap 3 detik sekali
        if current_time - last_media_check > 3:
            current_music = asyncio.run(get_windows_media_info())
            if len(current_music) > 40:
                current_music = current_music[:37] + "..."
            
            payload = f"MSC:{current_music}\n"
            esp32.write(payload.encode('utf-8'))
            
            if current_music != last_music:
                print(f"🎵 [MEDIA UPDATE] Now Playing: {current_music}")
                last_music = current_music
                
            last_media_check = current_time

        # Membaca sinyal masuk dari MACROPADS
        if esp32.in_waiting > 0:
            data = esp32.readline().decode('utf-8').strip()
            if not data:
                continue

            # --------------------------------------------------------------------
            # 1. MONITOR UTAMA: MATRIKS TOMBOL 3x3 (SHORTCUT PRODUKTIVITAS)
            # --------------------------------------------------------------------
            
            # --- BARIS 0 (Baris Atas) ---
            if data == "BTN:0,0":
                pyautogui.hotkey('ctrl', 'c')
                print("✅ [TOMBOL 1] Terdeteksi (B:0, K:0) ➔ MENGEKSEKUSI SHORTCUT: [ Ctrl + C ] (Copy)")
                
            elif data == "BTN:0,1":
                pyautogui.hotkey('ctrl', 'v')
                print("✅ [TOMBOL 2] Terdeteksi (B:0, K:1) ➔ MENGEKSEKUSI SHORTCUT: [ Ctrl + V ] (Paste)")
                
            # NOTE: BTN:0,2 (Ctrl + X) TELAH DIHAPUS KARENA SWITCH RUSAK / UPDATE LAYOUT PHYSICAL

            # --- BARIS 1 (Baris Tengah) ---
            elif data == "BTN:1,0":
                pyautogui.hotkey('ctrl', 'z')
                print("✅ [TOMBOL 3] Terdeteksi (B:1, K:0) ➔ MENGEKSEKUSI SHORTCUT: [ Ctrl + Z ] (Undo)")
                
            elif data == "BTN:1,1":
                pyautogui.hotkey('ctrl', 'shift', 'z')
                print("✅ [TOMBOL 4] Terdeteksi (B:1, K:1) ➔ MENGEKSEKUSI SHORTCUT: [ Ctrl + Shift + Z ] (Redo)")
                
            elif data == "BTN:1,2" or data == "BTN:ENC":
                # Menangani fleksibilitas kiriman data dari klon klik fisik encoder Anda
                pyautogui.press('playpause')
                print("✅ [KLIK ENCODER] Terdeteksi (B:1, K:2) ➔ MENGEKSEKUSI SHORTCUT: [ Play / Pause Media ]")

            # --- BARIS 2 (Baris Bawah) ---
            elif data == "BTN:2,0":
                pyautogui.hotkey('ctrl', 'alt', 'tab')
                print("✅ [TOMBOL 5] Terdeteksi (B:2, K:0) ➔ MENGEKSEKUSI SHORTCUT: [ Ctrl + Alt + Tab ] (Switch Apps Menu)")
                
            elif data == "BTN:2,1":
                pyautogui.hotkey('alt', 'shift')
                print("✅ [TOMBOL 6] Terdeteksi (B:2, K:1) ➔ MENGEKSEKUSI SHORTCUT: [ Alt + Shift ] (Ubah Layout Keyboard)")
                
            elif data == "BTN:2,2":
                pyautogui.hotkey('ctrl', 'x') # Slot tombol ke-7: Show Desktop
                print("✅ [TOMBOL 7] Terdeteksi (B:2, K:2) ➔ MENGEKSEKUSI SHORTCUT: [ Ctrl + X ] (Cut)")

            # --------------------------------------------------------------------
            # 2. MONITOR PENDUKUNG: ENCODER ROTASI & SLIDER
            # --------------------------------------------------------------------
            elif data == "ENC:CW":
                pyautogui.press('nexttrack')
                print("🔄 [ROTASI ENCODER] Putar Kanan (CW) ➔ Aksi: Lompat ke Lagu Berikutnya (Next Track)")
                
            elif data == "ENC:CCW":
                pyautogui.press('prevtrack')
                print("↩️ [ROTASI ENCODER] Putar Kiri (CCW)  ➔ Aksi: Kembali ke Lagu Sebelumnya (Prev Track)")

            elif data.startswith("SLD:"):
                pld = data.split(":")[1]
                s_num, s_val = pld.split(",")
                s_val = max(0, min(int(s_val), 4095))
                persentase = int((s_val / 4095.0) * 100)

                if s_num == "1":
                    volume_control.SetMasterVolumeLevelScalar(s_val / 4095.0, None)
                    print(f"🎚️ [SLIDER 1] Mengirim Data Analog ➔ Aksi: Mengubah Volume Laptop ke {persentase}%")
                elif s_num == "2":
                    brg = int((s_val / 4095.0) * 100)
                    sbc.set_brightness(max(5, brg))
                    print(f"🔆 [SLIDER 2] Mengirim Data Analog ➔ Aksi: Mengubah Kecerahan Layar ke {max(5, brg)}%")

    except KeyboardInterrupt:
        print("\n[INFO] Program dihentikan oleh pengguna.")
        esp32.close()
        break
    except Exception as e:
        print(f"⚠️ [PERINGATAN] Kendala pembacaan data: {e}")
        time.sleep(0.1)