# seven7segment

## Connection

<img src="https://drive.google.com/uc?export=view&id=1BwUeDYt_xN0IrdARrumfaPBQBaIcosic">

## Usage

**Compiling**
```
cd /path/to/driver
make
```

**Instaling module**
```
sudo insmod seven7segment.ko
```

**Changing digit** (from 0 to 9)
```
cd /sys/seven7segment/seven7segment
sudo chmod a+rw current_digit
sudo echo "1" > current_digit
```

**Changing mode** (active/sleeping)
```
sudo chmod a+rw mode
// Sleeping mode
sudo echo "s" > mode
// Active mode
sudo echo "a" > mode
```

**Uninstalling module**
```
sudo rmmod seven7segment
```