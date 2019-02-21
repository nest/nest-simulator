
# NESTıo

* Each recording device can use several recording backends. The
  recording backends for a device are set by supplying a list of
  recording backend names as property \record_to.
* The `RecordingBackend` base class provides a `std::map<index,
  DictionaryDatum>` for storing the properties of the device in the
  recording backend.

How to set/retrieve general per-recording-backend properties and
per-device recording-backend settings



RB::register_int(std::string)
RB::register_double(std::string)
RB::write_int(data)
RB::write_int(data)



## Devices

### `spike_detector`

```
time  sender
```

# per recording device:

## ASCII

## Memory

## SionLib


### `multimeter`

```
time  sender  data1  data2  ...
```

### `weight_recorder`

```
time  sender  target  port  rport  weight
```


### No conversion for `PseudoRecordingDevices` required


## ASCII backend

- Write a descriptive header
- Write time in steps + offset
- Always have full precision
- Always writes the following columns:

time  sender  val1  val2 ...

### per recording backend properties:

- file_extension
- precision


## Screen backend

- Write a descriptive header
- Write time in double,
- Allow to set the precision of time and values (default to 3 digits)

## per recording device properties:

- precision

## Sionlib backend

## per recording device properties:

- 


## Format 



ID Map

id  num_ints ints

    gid (source gid)
	connection (source gid, target gid, port, rport, target vp)
	gid pair (source gid, target gid)
	

 
