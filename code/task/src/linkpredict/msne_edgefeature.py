import numpy as np
from gensim.models import Word2Vec
from gensim.models.keyedvectors import EuclideanKeyedVectors
import time


class MSNEEdgeFeature(object):

    def __init__(self, nxG=None):
        self.nxG = nxG
        self.G  = None
        self.model = None


    def learn_embeddings(self,walks, d, k):
        self.model = Word2Vec(walks, size=d, window=k, min_count=0, sg=1, workers=2, iter=1)
        return

    def fit(self, walksfile, d=128, k=10 ):
        start_time_fit = time.time()
        walks = []
        file = open(walksfile)
        for line in file:
            walks.append(line.split(","))
        self.learn_embeddings(walks, d, k)
        print("Total time for fit()", time.time()-start_time_fit, "seconds")


    def select_operator_from_str(self, binary_operator):
        if binary_operator == 'l1':
            return self.operator_l1
        elif binary_operator == 'l2':
            return self.operator_l2
        elif binary_operator == 'avg':
            return self.operator_avg
        elif binary_operator == 'h':
            return self.operator_hadamard

        print("CAUTION: Operator"+binary_operator+"is invalid. Returning Hadamard operator.")
        return self.operator_hadamard

    def operator_hadamard(self, u, v):
        return u*v

    def operator_avg(self, u, v):
        return (u+v)/2.0

    def operator_l2(self, u, v):
        return (u-v)**2

    def operator_l1(self, u, v):
        return np.abs(u-v)

    def transform(self, data_edge, binary_operator = 'h'):
        X = []
        y = []
        func_bin_operator = self.select_operator_from_str(binary_operator)

        for row in data_edge:
            y.append(row[-1])
            u_str = str(row[0])
            v_str = str(row[1])
            if type(self.model) is Word2Vec or type(self.model) is EuclideanKeyedVectors:
                X.append(func_bin_operator(self.model[u_str], self.model[v_str]))
            else:
                X.append(func_bin_operator(self.model.loc[u_str],self.model.loc[v_str]))

        return np.array(X), np.array(y)