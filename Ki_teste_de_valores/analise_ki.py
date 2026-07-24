import pandas as pd
import matplotlib.pyplot as plt

# 1. Carregar os dados
nome_ficheiro = 'dados_ki.csv'

try:
    df = pd.read_csv(nome_ficheiro)
except FileNotFoundError:
    print(f"Erro: O ficheiro '{nome_ficheiro}' não foi encontrado na mesma pasta do script.")
    exit()

# Converter tempo para segundos relativos a partir de t = 0s
df['Tempo_s'] = (df['Tempo_ms'] - df['Tempo_ms'].iloc[0]) / 1000.0

# Estilo gráfico limpo
plt.style.use('seaborn-v0_8-whitegrid' if 'seaborn-v0_8-whitegrid' in plt.style.available else 'default')

# ==============================================================================
# FIGURA 1: VISÃO CONTINUADA DOS 35 SEGUNDOS (RPM e PWM)
# ==============================================================================
fig1, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 8), sharex=True)

# Plot 1.1: RPM vs Setpoint
ax1.plot(df['Tempo_s'], df['Setpoint'], 'r--', label='Setpoint (100 RPM)', linewidth=2)
ax1.plot(df['Tempo_s'], df['RPM_Medido'], 'b-', label='RPM Medido', linewidth=1.5)
ax1.set_ylabel('Velocidade (RPM)', fontsize=11, fontweight='bold')
ax1.set_title('Ensaio de Resposta do Controlador PI (Ki Variável)', fontsize=14, fontweight='bold')
ax1.legend(loc='upper right', frameon=True)
ax1.grid(True, linestyle='--', alpha=0.7)

# Plot 1.2: Sinal de Controlo (PWM)
ax2.plot(df['Tempo_s'], df['PWM_Out'], color='darkgreen', label='Sinal PWM (0-255)', linewidth=1.5)
ax2.set_xlabel('Tempo (segundos)', fontsize=11, fontweight='bold')
ax2.set_ylabel('PWM Output', fontsize=11, fontweight='bold')
ax2.set_title('Atuação no Motor (Sinal de Controlo PWM)', fontsize=12, fontweight='bold')
ax2.legend(loc='upper right', frameon=True)
ax2.grid(True, linestyle='--', alpha=0.7)

# Identificar as 5 fases de Ki no gráfico
ensaios = df['Ensaio'].unique()
for ensaio in ensaios:
    df_sub = df[df['Ensaio'] == ensaio]
    t_inicio = df_sub['Tempo_s'].iloc[0]
    ki_val = df_sub['Ki'].iloc[0]

    # Linhas verticais divisórias
    ax1.axvline(x=t_inicio, color='black', linestyle=':', alpha=0.6)
    ax2.axvline(x=t_inicio, color='black', linestyle=':', alpha=0.6)

    # Anotação do Ki no gráfico superior
    ax1.text(t_inicio + 0.3, ax1.get_ylim()[0] + 12, f'Ki = {ki_val:.2f}', 
             fontweight='bold', bbox=dict(boxstyle='round,pad=0.3', facecolor='yellow', alpha=0.5))

plt.tight_layout()

# ==============================================================================
# FIGURA 2: COMPARAÇÃO SOBREPOSTA DOS 5 Ki (0 a 5s)
# ==============================================================================
fig2, ax3 = plt.subplots(figsize=(10, 6))

cores = ['tab:blue', 'tab:orange', 'tab:green', 'tab:red', 'tab:purple']

for i, ensaio in enumerate(ensaios):
    df_sub = df[df['Ensaio'] == ensaio].copy()
    # Reiniciar tempo de cada ensaio para arrancar em 0 segundos
    t_relativo = (df_sub['Tempo_ms'] - df_sub['Tempo_ms'].iloc[0]) / 1000.0
    ki_val = df_sub['Ki'].iloc[0]

    ax3.plot(t_relativo, df_sub['RPM_Medido'], label=f'Ki = {ki_val:.2f}', color=cores[i], linewidth=2)

# Setpoint de referência na comparação
ax3.axhline(y=100.0, color='red', linestyle='--', label='Setpoint (100 RPM)', linewidth=2)

ax3.set_xlabel('Tempo do Ensaio (segundos)', fontsize=11, fontweight='bold')
ax3.set_ylabel('Velocidade (RPM)', fontsize=11, fontweight='bold')
ax3.set_title('Comparação Direta de Resposta para Diferentes Valores de Ki', fontsize=13, fontweight='bold')
ax3.legend(loc='lower right', frameon=True)
ax3.grid(True, linestyle='--', alpha=0.7)

plt.tight_layout()

# Exibir os gráficos no ecrã
plt.show()