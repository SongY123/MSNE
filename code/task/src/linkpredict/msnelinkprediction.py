import networkx as nx
from linkpredict.msne_edgefeature import MSNEEdgeFeature
import pandas as pd
from sklearn.linear_model import LogisticRegressionCV
from sklearn.model_selection import train_test_split
from sklearn.metrics import roc_auc_score
from sklearn.preprocessing import StandardScaler


def link_prediction(G,  edge_data_filename, walksfile, embfile, d=128, k=10, binary_operators = ['h']):
    edge_data = pd.read_csv(edge_data_filename, delimiter=' ', header=None).as_matrix()
    edge_data = edge_data.astype(int)
    msne_edgefeature = MSNEEdgeFeature(G)
    msne_edgefeature.fit(walksfile, embfile, d, k)

    scores = []

    sc = StandardScaler()
    for binary_operator in binary_operators:
        X,y = msne_edgefeature.transform(edge_data, binary_operator)
        X_train, X_test, y_train, y_test = train_test_split(X, y, train_size=0.5, test_size="default", random_state=100)
        X_train = sc.fit_transform(X_train)
        X_test = sc.transform(X_test)
        clf = LogisticRegressionCV(Cs=10, cv=10, scoring='roc_auc', verbose=True)
        clf.fit(X_train, y_train)

        y_pred = clf.predict_proba(X_test)
        if clf.classes_[0] == 1:
            score_auc = roc_auc_score(y_test, y_pred[:, 0])
        else:
            score_auc = roc_auc_score(y_test, y_pred[:, 1])

        print("Score:", score_auc)
        scores.append(score_auc)
    return scores

def read_graph(graph_filename):
    G = nx.read_edgelist(graph_filename, nodetype=int, create_using=nx.DiGraph())
    for edge in G.edges():
        G[edge[0]][edge[1]]['weight'] = 1
    G = G.to_undirected()
    return G