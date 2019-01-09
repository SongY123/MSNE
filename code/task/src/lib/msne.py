from gensim.models import Word2Vec


class MSNE(object):

    def __init__(self, graph, walks, path_length, num_paths, dim, **kwargs):
        
        kwargs["workers"] = kwargs.get("workers", 1)

        self.graph = graph
        sentences = []
        file = open(walks)
        for line in file:
            sentences.append(line.strip().split(","))
        kwargs["sentences"] = sentences
        kwargs["min_count"] = kwargs.get("min_count", 0)
        kwargs["size"] = kwargs.get("size", dim)
        kwargs["sg"] = 1

        self.size = kwargs["size"]
        print ("Learning representation...")
        word2vec = Word2Vec(**kwargs)
        self.vectors = {}
        for word in graph.G.nodes():
            self.vectors[word] = word2vec[word]
        del word2vec

    def save_embeddings(self, filename):
        fout = open(filename, 'w')
        node_num = len(self.vectors.keys())
        fout.write("{} {}\n".format(node_num, self.size))
        for node, vec in self.vectors.items():
            fout.write("{} {}\n".format(node,
                                        ' '.join([str(x) for x in vec])))
        fout.close()
