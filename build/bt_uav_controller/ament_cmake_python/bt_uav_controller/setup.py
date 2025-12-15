from setuptools import find_packages
from setuptools import setup

setup(
    name='bt_uav_controller',
    version='0.0.0',
    packages=find_packages(
        include=('bt_uav_controller', 'bt_uav_controller.*')),
)
