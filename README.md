# CVRandomWalks
Fast tool to obtain sentences based on random walks from networks for use in word2vec and other embedding techniques.  

## Requirements
- C11 compatible compiler (GCC, CLANG)
- OpenMP or libdispatch (MacOS)

## Compiling
Makefile can be used to compile the tool, just type

```
> cd CVRandomWalks
> make
```

The binary files will be generated at `Build_($platform)`.

## Usage
The walking algorithm uses regular random walks and biased random walks according to node2vec ([https://snap.stanford.edu/node2vec/]).

```
./CVRandomWalks q p w m network.xnet sentences.txt
```

- `p` - for p in node2vec walks (1.0 for unbiased) [positive number]
- `q` - for q in node2vec walks (1.0 for unbiased) [positive number]
- `w` - length of each walk [positive integer]
- `m` - of sentences to be generated [positive integer]

Input is in `.xnet` format like in [http://github.com/filipinascimento/xnet]. For python, you can use the `xnetwork` package.

The output is a text with words separated by space and sentences separated new line. It can be used directly on 

## TODO
- Build a python wrapper based on igraph and/or networkx
- Include other kinds of walks
- Better help
- Error checking
