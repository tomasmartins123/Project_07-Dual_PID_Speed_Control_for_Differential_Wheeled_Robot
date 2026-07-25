import pandas as pd
import matplotlib.pyplot as plt

# 1. Load data
file_name = 'pid_data.csv'

try:
    df = pd.read_csv(file_name)
except FileNotFoundError:
    print(f"Error: The file '{file_name}' was not found in the script directory.")
    exit()

# Convert time to relative seconds starting from t = 0s
df['Time_s'] = (df['Time_ms'] - df['Time_ms'].iloc[0]) / 1000.0

# Clean plot style
plt.style.use('seaborn-v0_8-whitegrid' if 'seaborn-v0_8-whitegrid' in plt.style.available else 'default')

# Create figure with 2 vertical subplots
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 8), sharex=True)

# ==============================================================================
# SUBPLOT 1: RPM COMPARISON (Left Wheel vs Right Wheel vs Setpoint)
# ==============================================================================
ax1.plot(df['Time_s'], df['Setpoint'], 'r--', label='Setpoint (100 RPM)', linewidth=2)
ax1.plot(df['Time_s'], df['Left_RPM'], label='Left Wheel', color='tab:blue', linewidth=1.8)
ax1.plot(df['Time_s'], df['Right_RPM'], label='Right Wheel', color='tab:green', linewidth=1.8)

ax1.set_ylabel('Speed (RPM)', fontsize=11, fontweight='bold')
ax1.set_title('Dual PID Controller Performance (Left vs Right Wheel)', fontsize=14, fontweight='bold')
ax1.legend(loc='upper right', frameon=True)
ax1.grid(True, linestyle='--', alpha=0.7)

# ==============================================================================
# SUBPLOT 2: CONTROL SIGNAL COMPARISON (Left PWM vs Right PWM)
# ==============================================================================
ax2.plot(df['Time_s'], df['Left_PWM'], label='Left PWM', color='tab:blue', linewidth=1.5)
ax2.plot(df['Time_s'], df['Right_PWM'], label='Right PWM', color='tab:green', linewidth=1.5)

ax2.set_xlabel('Time (seconds)', fontsize=11, fontweight='bold')
ax2.set_ylabel('PWM Output (0-255)', fontsize=11, fontweight='bold')
ax2.set_title('Motor Actuation Effort (PWM Control Signals)', fontsize=12, fontweight='bold')
ax2.legend(loc='upper right', frameon=True)
ax2.grid(True, linestyle='--', alpha=0.7)

plt.tight_layout()

# Save the dual plot to file
plt.savefig('pid_performance.png', dpi=300, bbox_inches='tight')

# Display plots on screen
plt.show()