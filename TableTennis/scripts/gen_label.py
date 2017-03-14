import sys
import numpy as np
import lmdb
import caffe
import argparse
import pandas as pd

def parse_args():
    """Parse input arguments."""
    parser = argparse.ArgumentParser(description='generate multi-label file')
    parser.add_argument('--view', dest='view', help='lmdb file to view')
    parser.add_argument('--file', dest='file', help='ground truth file')
    parser.add_argument('--outname', dest='outname', help='output lmdb file name')

    args = parser.parse_args()

    return args

if __name__ == '__main__':
	args = parse_args()
	view = args.view
	filename = args.file
	outname = args.outname

	if not view == None:
		env = lmdb.open(view, readonly=True)
		with env.begin() as txn:
			cursor = txn.cursor()
			for key, value in cursor:
				datum = caffe.proto.caffe_pb2.Datum()
				datum.ParseFromString(value)
				print datum.channels, datum.height, datum.width
				flat_x = np.fromstring(datum.data, dtype=np.int64)
				print flat_x.shape
				x = flat_x.reshape(datum.channels, datum.height, datum.width)
				y = datum.label
				print x
		env.close()

	else:

		data = pd.read_csv(filename, delimiter=' ')
		arr = data.as_matrix(['cls', 'x1', 'y1', 'x2', 'y2'])
		data = None
		N, D = arr.shape

		key = 0
		env = lmdb.open(outname, map_size=arr.nbytes*10)

		with env.begin(write=True) as txn:
			for i in range(N):
				data = arr[i,:]
				datum = caffe.proto.caffe_pb2.Datum()
				datum.channels = arr.shape[1]
				datum.height = 1
				datum.width = 1
				datum.data = data.tostring()
				print data
				datum.label = 0
				key_str = '{:08}'.format(key)

				txn.put(key_str.encode('ascii'), datum.SerializeToString())
				key+=1
		env.close()

