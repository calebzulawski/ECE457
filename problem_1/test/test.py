#!/usr/bin/env python3

import urllib.request
import timeit

def var2fun(x):
	return 'subprocess.call([\'' + '\',\''.join(x) + '\'], stdout=subprocess.DEVNULL)'

if __name__ == '__main__':
	bin = '../src/copycat'
	txt = 'mobydick.txt'
	concat1 = [bin, txt]
	concat3 = [bin, txt, txt, txt]

	urllib.request.urlretrieve("http://www.gutenberg.org/cache/epub/2701/pg2701.txt", txt)

	print(timeit.Timer(var2fun(concat1), setup="import subprocess").repeat(repeat=5))