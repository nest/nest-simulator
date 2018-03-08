
# NESTıo

* Each device can use exactly one recording backend.
* The backend is selected by setting the property `backend` to the name of the backend (as **string** or **literal**).
* All recorder properties are stored in the recording backend.
* The `RecordingBackend` base class provides a `std::map<index, DictionaryDatum>` for this.




RB::register_int(std::string)
RB::register_double(std::string)
RB::write_int(data)
RB::write_int(data)



## Devices

### `spike_detector`

```
time  sender
```

### `multimeter`

```
time  sender  data1  data2  ...
```

### `weight_recorder`

```
time  sender  target  port  rport  weight
```


### No conversion for `PseudoRecordingDevices`


## ASCII backend

- Write a descriptive header
- Write time in steps + offset
- Always have full precision
- Always writes the following columns:

time  sender  val1  val2 ...

## Screen backend

- Write a descriptive header
- Write time in double,
- Allow to set the precision of time and values (default to 3 digits)




## Format 



ID Map

id  num_ints ints

    gid (source gid)
	connection (source gid, target gid, port, rport, target vp)
	gid pair (source gid, target gid)
	

 
