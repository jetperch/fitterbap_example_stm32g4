#!/usr/bin/env python3
# Copyright 2021 Jetperch LLC
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

"""
Update all UART files from one source.
"""

import os
import re
import argparse


MYPATH = os.path.dirname(os.path.abspath(__file__))
H_FILE = os.path.join(MYPATH, 'Core', 'Inc', 'uart{id}.h')
C_FILE = os.path.join(MYPATH, 'Core', 'Src', 'uart{id}.c')
UART_INFO = {
    # name, uart, dma, dma_ch_rx, dma_ch_tx
    1: ('USART1', 1, 1, 1, 2),
    2: ('USART2', 2, 1, 3, 4),
    3: ('USART3', 3, 1, 5, 6),
    4: ('UART4',  4, 1, 7, 8),
    5: ('UART5',  5, 2, 1, 2),
}

def _re_table(uart_src, uart_dst):
    src_name, src_id, src_dma, src_dma_ch_rx, src_dma_ch_tx = UART_INFO[uart_src]
    dst_name, dst_id, dst_dma, dst_dma_ch_rx, dst_dma_ch_tx = UART_INFO[uart_dst]
    clock_by_port = {
        1: 'LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1)',
        2: 'LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2)',
        3: 'LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3)',
        4: 'LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART4)',
        5: 'LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART5)',
    }
    
    lookup = [
        (re.escape(clock_by_port[src_id]), clock_by_port[dst_id]),
        (f'uart{src_id}', f'uart{dst_id}'),
        (f'UART{src_id}', f'UART{dst_id}'),
        (f'usart{src_id}', f'usart{dst_id}'),
        (f'USART{src_id}', f'USART{dst_id}'),
        (f'DMA{src_dma}', f'DMA{dst_dma}'),
        ('USART4', 'UART4'),
        ('USART5', 'UART5'),
    ]
    for ch_src, ch_dst in [(src_dma_ch_rx, dst_dma_ch_rx), (src_dma_ch_tx, dst_dma_ch_tx)]:
        lookup.append((rf'DMA{src_dma}_Channel{ch_src}_', rf'DMA{dst_dma}_Channel{ch_dst}_'))
        lookup.append((f'LL_DMA_CHANNEL_{ch_src}', f'LL_DMA_CHANNEL_{ch_dst}'))
        lookup.append((rf'LL_DMA_ClearFlag_([a-zA-Z]+){ch_src}', rf'LL_DMA_ClearFlag_\g<1>{ch_dst}'))
        lookup.append((rf'LL_DMA_IsActiveFlag_([a-zA-Z]+){ch_src}', rf'LL_DMA_IsActiveFlag_\g<1>{ch_dst}'))
        lookup.append((rf'DMA_IFCR_(\S+){ch_src}', rf'DMA_IFCR_\g<1>{ch_dst}'))
    return lookup


def _re(s, lookup):
    for src, dst in lookup:
        s = re.sub(src, dst, s)
    return s


def _sub(lookup, file_pairs):
    for fname_in, fname_out in file_pairs:
        with open(fname_in, encoding='utf-8') as f:
            s = f.read()
        s = _re(s, lookup)
        with open(fname_out, 'w', encoding='utf-8') as f:
            f.write(s)


def get_parser():
    p = argparse.ArgumentParser(
        description='Generate UART files from a single template.',
        epilog='To update UARTs 2, 3, 4, and 5 from 1: '
               'python3 uart_gen.py 1 2 3 4 5')
    p.add_argument('uart_src',
                   type=int,
                   help='The source UART identifier.')
    p.add_argument('uart_dst',
                   type=int,
                   nargs='+',
                   help='The destination UART identifier.')
    return p


def run():
    parser = get_parser()
    args = parser.parse_args()
    for uart_dst in args.uart_dst:
        uart_src = args.uart_src
        print(f'Processing {uart_src} to {uart_dst}')
        _re_map = _re_table(uart_src, uart_dst)
        files = [(H_FILE.format(id=uart_src), H_FILE.format(id=uart_dst)),
                 (C_FILE.format(id=uart_src), C_FILE.format(id=uart_dst))]
        _sub(_re_map, files)
    return 0


if __name__ == '__main__':
    run()
