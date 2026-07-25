import pandas as pd
import matplotlib.pyplot as plt

# 1. Load data
file_name = 'kp_data.csv'

try:
    df = pd.read_csv(file_name)
except FileNotFoundError:
    print(f"Error: The file '{file_name}' was not found in the script directory.")
    exit()

# Convert time to relative seconds starting from t = 0s
df['Time_s'] = (df['Time_ms'] - df['Time_ms'].iloc[0]) / 1000.0

# Clean plot style
plt.style.use('seaborn-v0_8-whitegrid' if 'seaborn-v0_8-whitegrid' in plt.style.available else 'default')

# ==============================================================================
# FIGURE 1: CONTINUOUS OVERVIEW OF TESTS (RPM AND PWM)
# ==============================================================================
fig1, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 8), sharex=True)

# Plot 1.1: RPM vs Setpoint
ax1.plot(df['Time_s'], df['Setpoint'], 'r--', label='Setpoint (100 RPM)', linewidth=2)
ax1.plot(df['Time_s'], df['Measured_RPM'], 'b-', label='Measured RPM', linewidth=1.5)
ax1.set_ylabel('Speed (RPM)', fontsize=11, fontweight='bold')
ax1.set_title('Proportional Controller Response Test (Variable Kp)', fontsize=14, fontweight='bold')
ax1.legend(loc='upper right', frameon=True)
ax1.grid(True, linestyle='--', alpha=0.7)

# Plot 1.2: Control Signal (PWM)
ax2.plot(df['Time_s'], df['PWM_Out'], color='darkgreen', label='PWM Signal (0-255)', linewidth=1.5)
ax2.set_xlabel('Time (seconds)', fontsize=11, fontweight='bold')
ax2.set_ylabel('PWM Output', fontsize=11, fontweight='bold')
ax2.set_title('Motor Actuation (PWM Control Signal)', fontsize=12, fontweight='bold')
ax2.legend(loc='upper right', frameon=True)
ax2.grid(True, linestyle='--', alpha=0.7)

# Identify the 5 Kp test phases in the graph
trials = df['Trial'].unique()
for trial in trials:
    df_sub = df[df['Trial'] == trial]
    start_time = df_sub['Time_s'].iloc[0]
    kp_val = df_sub['Kp'].iloc[0]

    # Vertical divider lines
    ax1.axvline(x=start_time, color='black', linestyle=':', alpha=0.6)
    ax2.axvline(x=start_time, color='black', linestyle=':', alpha=0.6)

    # Kp annotation on the top plot
    ax1.text(start_time + 0.3, ax1.get_ylim()[0] + 12, f'Kp = {kp_val:.2f}', 
             fontweight='bold', bbox=dict(boxstyle='round,pad=0.3', facecolor='yellow', alpha=0.5))

plt.tight_layout()

# ==============================================================================
# FIGURE 2: OVERLAID COMPARISON OF THE 5 Kp VALUES
# ==============================================================================
fig2, ax3 = plt.subplots(figsize=(10, 6))

colors = ['tab:blue', 'tab:orange', 'tab:green', 'tab:red', 'tab:purple']

for i, trial in enumerate(trials):
    df_sub = df[df['Trial'] == trial].copy()
    # Reset time for each trial to start at 0 seconds
    relative_time = (df_sub['Time_ms'] - df_sub['Time_ms'].iloc[0]) / 1000.0
    kp_val = df_sub['Kp'].iloc[0]

    ax3.plot(relative_time, df_sub['Measured_RPM'], label=f'Kp = {kp_val:.2f}', color=colors[i], linewidth=2)

# Reference Setpoint for comparison
ax3.axhline(y=100.0, color='red', linestyle='--', label='Setpoint (100 RPM)', linewidth=2)

ax3.set_xlabel('Trial Duration (seconds)', fontsize=11, fontweight='bold')
ax3.set_ylabel('Speed (RPM)', fontsize=11, fontweight='bold')
ax3.set_title('Direct Response Comparison for Different Kp Values', fontsize=13, fontweight='bold')
ax3.legend(loc='lower right', frameon=True)
ax3.grid(True, linestyle='--', alpha=0.7)

plt.tight_layout()

# Save both images for the Kp test
fig1.savefig('kp_test_continuous.png', dpi=300, bbox_inches='tight')
fig2.savefig('kp_test_comparison.png', dpi=300, bbox_inches='tight')

# Display plots on screen
plt.show()