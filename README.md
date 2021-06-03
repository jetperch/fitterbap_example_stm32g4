<!--
# Copyright 2014-2021 Jetperch LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
-->

# Welcome!

Welcome to the [Fitterbap](https://github.com/jetperch/fitterbap_ 
STM32G4 example!  This example targets the STM32G4 microcontrollers from
STMicroelectronics.  The official release targets the NUCLEO-G491RE,
a $21 USD development board.


## NUCLEO-G491RE

The NUCLEO-G491RE development board contains the
STM32G431KB Cortex-M4 microcontroller with 128 kB RAM and 512 kB flash,
which provides plenty of room for a Fitterbap demonstration.  The
default code runs the Fitterbap comm protocol on 5 UARTS, simultaneously,
to allow for extensive testing of Fitterbap's distributed 
Publish-Subscribe (PubSub) implementation.

This demonstration uses the following USART ports:

| Port   | Mode       | TX pin | RX pin |
| ------ | ---------- | ------ | ------ |
| USART1 | Server     | PA9    | PA10   |
| USART2 | VCP Client | PA3    | PA2    |
| USART3 | Server     | PB10   | PB11   |
| USART4 | Client     | PC10   | PC11   |
| USART5 | Server     | PC12   | PD2    |

All ports are 3,000,000 baud, no parity, 8 data bits, 
and one stop bit (N81).

When wiring together two boards, connect a server port
to a client port.  Connect server.TX to client.RX and
server.RX to client.TX.

The ID pins allow you to select the board prefix:

| Bit  | Pin   | Nucleo Pin(s) |
| ---- | ----- | ------------- |
| 0    | PB0   | CN7.34, CN6.4 |
| 1    | PB1   | CN10.24       |
| 2    | PB2   | CN10.22       |

The software prefix is 'a' + the 3-bit ID value. 
All ID pins are configured with pull-down resistors to
default to 0.


## Licenses

All original code is under the permissive Apache 2.0 license.
However, this repo also contains ST code released under the 3-clause BSD
license.
