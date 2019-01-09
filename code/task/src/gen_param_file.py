import numpy as np
import argparse
from collections import OrderedDict

_DFT_PARA_LST = [0.25, 0.50, 1.0, 2.0, 4.0]
_DFT_FILE_NAME = 'param'


def parse_args():
    parser = argparse.ArgumentParser(
        prog="Generate Parameters Files",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument('-list-param', help='list param to search',
                        default=_DFT_PARA_LST, metavar='PARAS', nargs='+')
    parser.add_argument('-filename', help='the name of parameter files',
                        default=_DFT_FILE_NAME)
    parser.add_argument('-comment', help='whether to output comment file',
                        default=False, action='store_true')
    parser.add_argument('idxs', help='which param to be used', metavar='IDXS')
    args = parser.parse_args()
    return args


def parse_args_idxs(idxs):
    midxs = OrderedDict()
    for lvl, item in enumerate(idxs.split(';')):
        item = item.strip()
        midxs[lvl+1] = [int(idx.strip()) for idx in item.split(',')]
    helpstr = 'Please enter valid `idxs`, like `1`, `1; 1`, `1; 1, 2`.\n'
    for k, v in midxs.items():
        assert k == len(v) or 1 == len(v), helpstr
        if k != 1 and len(v) == 1:
            midxs[k] = v * k
    return midxs

#####################
# Prob
#####################

def norm_prob(p, q):
    raw = np.array([1/p, 1, 1/q])
    sum_ = raw.sum()
    return raw / sum_


def get_para_grid(lst=_DFT_PARA_LST):
    grid = [
        norm_prob(p, q)
        for p in lst
        for q in lst
    ]
    return grid


def get_prob_str_ln(grid, idxs):
    content = []
    for idx in idxs:
        item = str(grid[idx].tolist())[1:-1]
        content.append(item)
    return '; '.join(content)


def get_probs_str(grid, midxs):
    content = []
    for idxs in midxs.values():
        content.append(
            get_prob_str_ln(grid, idxs)
        )
    return '\n'.join(content)


##############
# Comment
##############

def get_prob_comment_ln(idxs, lst=_DFT_PARA_LST):
    lstlen = len(lst)
    ln = []
    for idx in idxs:
        pidx = int(idx / lstlen)
        qidx = int(idx % lstlen)
        ln.append('p={}, q={}'.format(lst[pidx], lst[qidx]))
    return '; '.join(ln)


def get_prob_comment(midxs, lst=_DFT_PARA_LST):
    content = []
    for i in range(1, len(midxs)+1):
        content.append(get_prob_comment_ln(midxs[i], lst))
    return '\n'.join(content)


def write_para_grid(filename, midxs, lst=_DFT_PARA_LST, comment=False):
    with open(filename, 'w') as fout:
        grid = get_para_grid(lst)
        content = get_probs_str(grid, midxs)
        fout.write(content)
        fout.write('\n')
    if comment:
        filename = filename + '.comment'
        with open(filename, 'w') as fout:
            fout.write(
                get_prob_comment(midxs, lst)
            )


if __name__ == '__main__':
    args = parse_args()
    midxs = parse_args_idxs(args.idxs)
    for k, v in args.__dict__.items():
        print('{}: {}'.format(k, v))
    if args.comment:
        print('Generating param-files and comment-files...')
    else:
        print('Generating param-files...')
    write_para_grid(args.filename, midxs, lst=args.list_param, comment=args.comment)
    print('Good Jobs :)')
