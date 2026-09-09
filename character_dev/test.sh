make -C /knote/character_dev
insmod /knote/character_dev/knote.ko
ls /dev 
sudo echo "hello world" > /dev/knote
sudo cat /dev/knote
rmmod /knote/character_dev/knote.ko
make -C /knote/character_dev clean
