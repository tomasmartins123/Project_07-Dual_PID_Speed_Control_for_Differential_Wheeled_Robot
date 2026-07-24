import pandas as pd
import matplotlib.pyplot as plt

# 1. Carregar os dados
nome_ficheiro = 'dados_pid.csv'

try:
    df = pd.read_csv(nome_ficheiro)
except FileNotFoundError:
    print(f"Erro: O ficheiro '{nome_ficheiro}' não foi encontrado na mesma pasta do script.")
    exit()

# Converter tempo para segundos relativos a partir de t = 0s
df['Tempo_s'] = (df['Tempo_ms'] - df['Tempo_ms'].iloc[0]) / 1000.0

# Estilo gráfico limpo
plt.style.use('seaborn-v0_8-whitegrid' if 'seaborn-v0_8-whitegrid' in plt.style.available else 'default')

# Criar a figura com 2 subplots verticais
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 8), sharex=True)

# ==============================================================================
# SUBPLOT 1: COMPARAÇÃO DE RPM (Roda Esquerda vs Roda Direita vs Setpoint)
# ==============================================================================
ax1.plot(df['Tempo_s'], df['Setpoint'], 'r--', label='Setpoint (100 RPM)', linewidth=2)
ax1.plot(df['Tempo_s'], df['RPM_Esq'], label='Roda Esquerda', color='tab:blue', linewidth=1.8)
ax1.plot(df['Tempo_s'], df['RPM_Dir'], label='Roda Direita', color='tab:green', linewidth=1.8)

ax1.set_ylabel('Velocidade (RPM)', fontsize=11, fontweight='bold')
ax1.set_title('Desempenho do Controlo PID Duplo (Esquerda vs Direita)', fontsize=14, fontweight='bold')
ax1.legend(loc='upper right', frameon=True)
ax1.grid(True, linestyle='--', alpha=0.7)

# ==============================================================================
# SUBPLOT 2: COMPARAÇÃO DO SINAL DE CONTROLO (PWM Esquerda vs PWM Direita)
# ==============================================================================
ax2.plot(df['Tempo_s'], df['PWM_Esq'], label='PWM Esquerda', color='tab:blue', linewidth=1.5)
ax2.plot(df['Tempo_s'], df['PWM_Dir'], label='PWM Direita', color='tab:green', linewidth=1.5)

ax2.set_xlabel('Tempo (segundos)', fontsize=11, fontweight='bold')
ax2.set_ylabel('PWM Output (0-255)', fontsize=11, fontweight='bold')
ax2.set_title('Esforço de Atuação dos Motores (Sinais PWM)', fontsize=12, fontweight='bold')
ax2.legend(loc='upper right', frameon=True)
ax2.grid(True, linestyle='--', alpha=0.7)

plt.tight_layout()

# Exibir os gráficos no ecrã
plt.show()