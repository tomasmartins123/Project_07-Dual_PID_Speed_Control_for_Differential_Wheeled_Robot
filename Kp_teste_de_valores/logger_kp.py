import serial
import time

# --- CONFIGURAÇÕES ---
PORTA_SERIAL = 'COM3' 
BAUD_RATE = 115200
NOME_FICHEIRO = 'dados_kp.csv'

try:
    print(f"A ligar à porta {PORTA_SERIAL}...")
    ser = serial.Serial(PORTA_SERIAL, BAUD_RATE, timeout=2)
    
    with open(NOME_FICHEIRO, mode='w', encoding='utf-8') as ficheiro:
        print(f"Ligação estabelecida! A gravar os dados em '{NOME_FICHEIRO}'...\n")
        
        while True:
            if ser.in_waiting > 0:
                linha = ser.readline().decode('utf-8', errors='ignore').strip()
                
                if not linha:
                    continue

                # Quando o Arduino sinaliza o fim dos 25 segundos de teste
                if "# FIM_TESTE" in linha:
                    print("\n-------------------------------------------")
                    print("TESTE CONCLUÍDO! Todos os dados foram guardados.")
                    print("-------------------------------------------")
                    break

                # Exibe na consola e grava no ficheiro CSV
                print(linha)
                ficheiro.write(linha + '\n')

except KeyboardInterrupt:
    print("\nRecolha interrompida pelo utilizador.")
except Exception as e:
    print(f"\nErro de Execução: {e}")
finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()
        print(f"Ficheiro '{NOME_FICHEIRO}' gerado com sucesso!")