Channel Latency on RIO w/ Thread Priority and Pinning: 13us

| Machine | Channel | Priority | Pin | Sleeps | Latency |
| ------- | ------- | -------- | --- | ------ | ------- |
| RIO     | Crossbeam | yes    | yes | yes    | ~13us   |
| Intel i5-4690K @ 3.9GHz| Crossbeam | yes | yes | yes | ~4us |

## TODO
* Compare to STL implementation?
* Remove yields and check nvcsw and nivcsw
