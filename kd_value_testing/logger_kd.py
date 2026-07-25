import serial
import time

# --- CONFIGURATION ---
SERIAL_PORT = 'COM3'
BAUD_RATE = 115200
FILE_NAME = 'kd_data.csv'

try:
    print(f"Connecting to port {SERIAL_PORT}...")
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=2)
    
    with open(FILE_NAME, mode='w', encoding='utf-8') as file:
        print(f"Connection established! Recording data to '{FILE_NAME}'...\n")
        
        while True:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                
                if not line:
                    continue

                # When Arduino signals the end of the test (supports both English and Portuguese tags)
                if "# END_TEST" in line or "# FIM_TESTE" in line:
                    print("\n-------------------------------------------")
                    print("TEST CONCLUDED! All data has been saved.")
                    print("-------------------------------------------")
                    break

                # Display in console and write to CSV file
                print(line)
                file.write(line + '\n')

except KeyboardInterrupt:
    print("\nData collection interrupted by user.")
except Exception as e:
    print(f"\nExecution Error: {e}")
finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()
        print(f"File '{FILE_NAME}' generated successfully!")