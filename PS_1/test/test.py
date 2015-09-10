#!/usr/bin/env python

#import urllib.request
import timeit
import matplotlib.pyplot as plt
import os

def var2fun(x):
	exe = 'os.system(\'' + ' '.join(x) + ' > /dev/null\')'
	print(exe)
	return exe

if __name__ == '__main__':
	bin = '../src/copycat'
	txt = 'mobydick.txt'
	trials = 25;

	#urllib.request.urlretrieve("http://www.gutenberg.org/cache/epub/2701/pg2701.txt", txt)

	buffers = []
	rates = []
	buffers_n = []
	size = os.stat(txt).st_size

	for exp in range(0,19):
		buffer = 2**exp
		time = timeit.Timer(var2fun([bin, '-b', str(buffer), txt]), setup="import os").timeit(number = trials)
		buffers.append( exp )
		rate = size/(time/trials)
		rates.append( rate )
		if buffer > 1024:
			buffers_n.append(str(buffer/1024) + 'KB')
		else:
			buffers_n.append(str(buffer) + 'B')

	plt.bar(buffers, rates, align='center')
	plt.xticks(buffers, buffers_n)
	plt.xlabel('Buffer size')
	plt.ylabel('Copy rate (MB/s)')
	plt.title('Copy rate by buffer size')

	plt.show()