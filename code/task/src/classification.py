from argparse import ArgumentParser, ArgumentDefaultsHelpFormatter
from sklearn.linear_model import LogisticRegression
import os
import gen_param_file
from lib.graph import *
from lib.classify import Classifier, read_node_label
from lib import msne
def parse_args():
    parser = ArgumentParser(formatter_class=ArgumentDefaultsHelpFormatter,
                            conflict_handler='resolve', description='Classification task of MSNE')

    parser.add_argument('--dataset', default='blogcatalog',
                                help='Name of dataset')
    parser.add_argument('--graph-format', default='adjlist',choices=['adjlist','edgelist'],
                        help='Type of dataset')
    parser.add_argument('--levels', default=4, type=int,
                        help='Maximum level of parameters')
    parser.add_argument('--walk-length', default=80, type=int,
                        help='Length of the random walk started at each node')
    parser.add_argument('--weighted', action='store_true',default=False,
                        help='Treat graph as weighted')
    parser.add_argument('--directed', action='store_true',default=False,
                        help='Treat graph as directed.')
    parser.add_argument('--workers', default=8, type=int,
                        help='Number of parallel processes.')
    parser.add_argument('--number-walks', default=10, type=int,
                        help='Number of random walks to start at each node')
    parser.add_argument('--representation-size', default=128, type=int,
                        help='Number of latent dimensions to learn for each node.')
    parser.add_argument('--window-size', default=10, type=int,
                        help='Window size of skipgram model.')
    parser.add_argument('--clf-ratio', default=0.5, type=float,
                        help='The ratio of training data in the classification')
    args = parser.parse_args()
    return args

def main(args):
    args = parse_args()

    project_path = os.path.abspath(os.path.join(os.getcwd(), '..'))
    args.input = project_path + '\\data\\' + args.dataset + '\\edges.txt'
    args.label_file = project_path + '\\data\\' + args.dataset + '\\labels.txt'
    args.walkcmd = project_path + '\\cmd\\' + args.dataset + '\\walks.exe'

    param_path = project_path + '\\cmd\\' + args.dataset + '\\param\\level'
    emb_path = project_path + '\\cmd\\' + args.dataset + '\\emb\\level'
    data_path = project_path + '\\cmd\\' + args.dataset + '\\data\\level'
    dataset_path = project_path+'\\cmd\\'+args.dataset

    best_param = ''
    best_result = {'micro': 0, 'macro': 0}
    #Generate parameters files
    for level in range(1, args.levels, 1):
        params=[]
        #Check if the directory exists or not
        if not os.path.exists(param_path + str(level)):
            os.makedirs(param_path + str(level))
        if not os.path.exists(emb_path+ str(level)):
            os.makedirs(emb_path + str(level))
        if not os.path.exists(data_path + str(level)):
            os.makedirs(data_path + str(level))

        #Create param files
        with open( project_path + '\\cmd\\' + args.dataset+'\\config-lvl'+str(level),'w') as file:
            for i in range(25):
                if i==0:
                    file.write('.\\param\\level'+str(level)+'\\'+str(i)+' .\\data\\level'+str(level)+'\\walk'+str(i))
                else:
                    file.write('\n.\\param\\level' + str(level) + '\\' + str(i) + ' .\\data\\level' + str(
                        level) + '\\walk' + str(i))

        if len(best_param) == 0:
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
                params.append(best_param+';'+p)
                midxs = gen_param_file.parse_args_idxs(best_param+';'+p)
                gen_param_file.write_para_grid(param_path + str(level) + '\\' + str(i), midxs, comment=True)
        print('Params:', params)
        print('Level %d param adjust:' % level)

        #Generate random walk
        args.param = param_path +str(level)+'\\'
        os.chdir(dataset_path)
        print(args.walkcmd+' *./config-lvl'+str(level)+' * '+str(args.number_walks)+' '+str(args.walk_length)+' ./init/step1.out '+str(args.levels) +' ./log')
        os.system(args.walkcmd+' *./config-lvl'+str(level)+' * '+str(args.number_walks)+' '+str(args.walk_length)+' ./init/step1.out '+str(args.levels) +' ./log' )

        #Generate embeddings for each node and find the best parameters of MENE
        for i in range(0,25,1):
            args.walks = data_path + str(level) + '\\walk'+str(i)
            args.output = emb_path + str(level) + '\\emb'+str(i)
            print('walk %d ,params: %s' %(i, params[i]))
            g = Graph()
            print('Reading Graphs...')
            if args.graph_format == 'adjlist':
                g.read_adjlist(filename=args.input)
            elif args.graph_format == 'edgelist':
                g.read_edgelist(filename=args.input, weighted=args.weighted, directed=args.directed)
            model = msne.MSNE(graph=g, walks=args.walks, path_length=args.walk_length,
                              num_paths=args.number_walks, dim=args.representation_size,
                              workers=args.workers, window=args.window_size)
            vectors = model.vectors
            X, Y = read_node_label(args.label_file)
            print('Training classifier using {:.2f}% nodes...'.format(args.clf_ratio * 100))
            clf = Classifier(vectors=vectors, clf=LogisticRegression())
            result = clf.split_train_evaluate(X, Y, args.clf_ratio)
            print(result)
            if (result['micro'] + result['macro'] > best_result['micro'] + best_result['macro']):
                best_param = params[i]
                best_result = result
        print('Best params:',best_param)
        print('Best result:',best_result)
        os.chdir(project_path)

if __name__ == '__main__':
    main(parse_args())
