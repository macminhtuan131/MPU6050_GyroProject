import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider

# --- 1. TẠO DỮ LIỆU MÔ PHỎNG (SYNTHETIC DATA) ---
num_points = 300
dt = 0.02 # Thời gian vòng lặp 0.02s (50Hz)
t = np.arange(0, num_points * dt, dt)

# Góc thực tế: Quét từ -150 độ đến +150 độ (Hàm sin biên độ 150)
true_angle = 150 * np.sin(2 * np.pi * 0.15 * t)

# Gia tốc kế: Góc thực tế + Nhiễu rung lắc cơ học (Random +- 25 độ)
noise = np.random.uniform(-25, 25, size=len(t))
accel_angle = true_angle + noise

# Con quay hồi chuyển: Đạo hàm của hàm Sin + Nhiễu trôi dạt (Drift 5 độ/s)
gyro_rate = 150 * (2 * np.pi * 0.15) * np.cos(2 * np.pi * 0.15 * t) + 5.0

# Góc Gyro thuần túy (Tích phân từ vận tốc góc)
gyro_angle = np.zeros_like(t)
gyro_angle[0] = true_angle[0]
for i in range(1, len(t)):
    gyro_angle[i] = gyro_angle[i-1] + gyro_rate[i] * dt

# --- 2. HÀM TÍNH TOÁN BỘ LỌC BÙ ---
def calculate_filtered(alpha):
    filtered = np.zeros_like(t)
    filtered[0] = true_angle[0]
    for i in range(1, len(t)):
        # Công thức cốt lõi: Angle = Alpha * (Angle_cũ + Gyro_Rate * dt) + (1 - Alpha) * Accel
        filtered[i] = alpha * (filtered[i-1] + gyro_rate[i] * dt) + (1 - alpha) * accel_angle[i]
    return filtered

# --- 3. CẤU HÌNH VẼ BIỂU ĐỒ (MATPLOTLIB) ---
fig, ax = plt.subplots(figsize=(12, 7))
plt.subplots_adjust(bottom=0.25) # Chừa khoảng trống bên dưới cho thanh trượt

# Vẽ các đường dữ liệu
ax.plot(t, true_angle, 'g--', label='Góc thực tế (True)', linewidth=2.5)
ax.plot(t, accel_angle, 'red', alpha=0.3, label='Gia tốc kế (Bị nhiễu rung)', linewidth=1)
ax.plot(t, gyro_angle, 'blue', alpha=0.5, label='Gyroscope (Bị trôi lệch)', linewidth=2)

# Đường Bộ lọc bù (Đường chúng ta sẽ cập nhật khi kéo slider)
init_alpha = 0.96
filtered_line, = ax.plot(t, calculate_filtered(init_alpha), color='darkorange', label='BỘ LỌC BÙ', linewidth=3)

# Định dạng biểu đồ
ax.set_ylim(-250, 250)
ax.set_xlabel('Thời gian (Giây)', fontsize=12)
ax.set_ylabel('Góc nghiêng (Độ)', fontsize=12)
ax.set_title('MÔ PHỎNG ĐỘNG LỰC HỌC BỘ LỌC BÙ (COMPLEMENTARY FILTER)', fontsize=14, fontweight='bold')
ax.legend(loc='upper right')
ax.grid(True, linestyle=':', alpha=0.7)

# --- 4. TẠO THANH TRƯỢT (SLIDER) TƯƠNG TÁC ---
ax_alpha = plt.axes([0.15, 0.1, 0.7, 0.03], facecolor='lightcyan')
alpha_slider = Slider(
    ax=ax_alpha,
    label='Hệ số Alpha (α): ',
    valmin=0.0,
    valmax=1.0,
    valinit=init_alpha,
    valstep=0.01,
    color='darkorange'
)

# Hàm cập nhật biểu đồ khi kéo thanh trượt
def update(val):
    current_alpha = alpha_slider.val
    # Tính toán lại mảng dữ liệu với alpha mới
    new_filtered_data = calculate_filtered(current_alpha)
    # Cập nhật đường đồ thị màu cam
    filtered_line.set_ydata(new_filtered_data)
    fig.canvas.draw_idle()

alpha_slider.on_changed(update)

# Thêm ghi chú hiển thị trên cửa sổ
plt.figtext(0.15, 0.03, "Kéo thanh trượt để thấy đường BỘ LỌC BÙ (Màu cam) thay đổi. \nChú ý: Ở biên độ lớn (150 độ), nếu Alpha > 0.99 thì đường màu Cam sẽ không bao giờ bám kịp đường Thực tế.", fontsize=10, color='gray')

plt.show()