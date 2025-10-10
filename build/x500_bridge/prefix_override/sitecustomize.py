import sys
if sys.prefix == '/usr':
    sys.real_prefix = sys.prefix
    sys.prefix = sys.exec_prefix = '/home/imamalghifary/ws_sensor_combined/install/x500_bridge'
