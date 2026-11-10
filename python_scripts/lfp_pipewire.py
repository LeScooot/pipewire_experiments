import matplotlib.pyplot as plt
import numpy as np
from scipy import signal

order = 5 
taps = order+1
cutoff = 5000

f_s = 48e3
Wn = cutoff/f_s * 2 * np. pi

sos = signal.iirfilter(order, Wn= cutoff, fs=f_s, btype='lowpass', output='sos')

print(f"SOS vector: {sos}")


w, h = signal.freqz_sos(sos, fs=f_s)

freq_axis = (w)
fig, ax1 = plt.subplots(tight_layout=True)
ax1.set_title(f"Frequency Response of {order} order IIR Filter" +
              f"($f_c={cutoff}$ rad/sample)")
ax1.axvline(cutoff, color='black', linestyle=':', linewidth=0.8)
ax1.plot(freq_axis, 20 * np.log10(abs(h)), 'C0')
ax1.set_ylabel("Amplitude in dB", color='C0')
ax1.set(xlabel="Frequency in Hz")
ax1.set_ylim([-80 ,10])

plt.show()
