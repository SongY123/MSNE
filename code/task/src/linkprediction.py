from argparse import ArgumentParser
import os
import networkx
import gen_param_file
import linkpredict.msnelinkprediction as linkprediction
def parse_args():
    parser = ArgumentParser(description='Run link prediction and node2vec parameter sensitivity analysis.')
    parser.add_argument('--levels', default=4, type=int,
                        help='Maximum level of parameters')
    parser.add_argument('--dataset', default='ca',
                        help='Name of dataset')
    parser.add_argument('--weighted', action='store_true', default=False,
                        help='Treat graph as weighted')
    parser.add_argument('--directed', action='store_true', default=False,
                        help='Treat graph as directed.')
    parser.add_argument('--walk-length', default=80, type=int,
                        help='Length of the random walk started at each node')
    parser.add_argument('--workers', default=8, type=int,
                        help='Number of parallel processes.')
    parser.add_argument('--number-walks', default=10, type=int,
                        help='Number of random walks to start at each node')
    parser.add_argument('--representation-size', default=128, type=int,
                        help='Number of latent dimensions to learn for each node.')
    parser.add_argument('--window-size', default=10, type=int,
                        help='Window size of skipgram model.')

    return parser.parse_args()

def main(args):
    args = parse_args()

    project_path = os.path.abspath(os.path.join(os.getcwd(), '..'))
    args.input_graph = project_path + '\\lp-cmd\\' + args.dataset + '\\init\\edge.txt'
    args.input_edge_data = project_path + '\\lp-cmd\\' + args.dataset + '\\init\\group'
    args.walkcmd = project_path + '\\lp-cmd\\' + args.dataset + '\\walks.exe'

    param_path = project_path + '\\lp-cmd\\' + args.dataset + '\\param\\level'
    emb_path = project_path + '\\lp-cmd\\' + args.dataset + '\\emb\\level'
    data_path = project_path + '\\lp-cmd\\' + args.dataset + '\\data\\level'
    dataset_path = project_path + '\\lp-cmd\\' + args.dataset

    args.method = 'msne'

    best_param = {'h': ''}
    best_result = {'h':0}

    flag = 0
    for key in best_result.keys():
        print('find best ',key)
        for level in range(1, args.levels, 1):
            params=[]

            if not os.path.exists(param_path + str(level)):
                os.makedirs(param_path + str(level))
            if not os.path.exists(emb_path + str(level)):
                os.makedirs(emb_path + str(level))
            if not os.path.exists(data_path + str(level)):
                os.makedirs(data_path + str(level))

            # Create param files
            with open( project_path + '\\lp-cmd\\' + args.dataset+'\\config-lvl'+str(level),'w') as file:
                for i in range(25):
                    if i==0:
                        file.write('.\\param\\level'+str(level)+'\\'+str(i)+' .\\data\\level'+str(level)+'\\walk'+str(i))
                    else:
                        file.write('\n.\\param\\level' + str(level) + '\\' + str(i) + ' .\\data\\level' + str(
                            level) + '\\walk' + str(i))

            if len(best_param[key]) == 0:
                for i in range(25):
                    params.append(str(i))
                    midxs = gen_param_file.parse_args_idxs(str(i))
                    gen_param_file.write_para_grid(param_path + str(level)+ '\\' +str(i), midxs, comment=True)
            else:
                for i in range(25):
                    p = ''
                    for _ in range(level):
                        p+=(str(i)+',')
                    p = p[0:len(p)-1]
                    params.append(best_param[key]+';'+p)
                    midxs = gen_param_file.parse_args_idxs(best_param[key]+';'+p)
                    gen_param_file.write_para_grid(param_path + str(level) + '\\' + str(i), midxs, comment=True)

            print('params:', params)
            print('Level %d param adjust:' % level)

            # Generate random walk
            args.param = param_path +str(level)+'\\'
            os.chdir(dataset_path)
            print(args.walkcmd+' *.\\config-lvl'+str(level)+' * '+str(args.number_walks)+' '+str(args.walk_length)+' .\\init\\step1.out '+str(args.levels) +' .\\log')
            if flag==0:
                os.system(args.walkcmd+' *.\\config-lvl'+str(level)+' * '+str(args.number_walks)+' '+str(args.walk_length)+' .\\init\\step1.out '+str(args.levels) +' .\\log' )

            G = linkprediction.read_graph(args.input_graph)

            if networkx.is_connected(G):
                print('G is connected')
            else:
                print('ERROR: G is not connected!')

            for i in range(25):
                args.walks = data_path + str(level) + '\\walk' + str(i)
                args.output = emb_path + str(level) + '\\emb' + str(i)
                print('walk %d ,params: %s' %(i, params[i]))
                result = linkprediction.link_prediction(G, args.input_edge_data, args.walks,args.output, args.representation_size, args.window_size, key)
                if(result[key]>best_result[key] or i==0):
                    best_param[key] = params[i]
                    best_result[key] = result[key]
            print('Best params:',best_param)
            print('Best result:',best_result)
            os.chdir(project_path)
        flag = 1

if __name__ == '__main__':
    main(parse_args())
