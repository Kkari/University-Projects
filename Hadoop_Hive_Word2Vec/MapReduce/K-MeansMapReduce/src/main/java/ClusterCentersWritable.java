import java.io.DataInput;
import java.io.DataOutput;
import java.io.EOFException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.concurrent.ThreadLocalRandom;

import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.WritableComparable;

public class ClusterCentersWritable implements Writable {

		VectorWritable[] center_vectors;
	
        public ClusterCentersWritable(VectorWritable[] center_vectors) {
            this.center_vectors = center_vectors;
        }

        public ClusterCentersWritable() {}
        
        public ClusterCentersWritable(int kClusters) {
        	center_vectors = new VectorWritable[kClusters];
        }
        
        public ClusterCentersWritable(int kClusters, int vectorLength, float min, float max) {
        	center_vectors = new VectorWritable[kClusters];
        	for(int i = 0; i < kClusters; i++) {
        		float vector[] = new float[vectorLength];
        		for(int j = 0; j < vectorLength; j++) {
        			vector[j] = (float)ThreadLocalRandom.current().nextDouble(min, max);
        		}
        		center_vectors[i] = new VectorWritable(vector);
        	}
        }
        
        public VectorWritable getCenter(int i) {
        	return center_vectors[i];
        }
        
        public void setCenter(int i, VectorWritable center) {
        	center_vectors[i] = center;
        }

        @Override
        public void write(DataOutput out) throws IOException {
        	out.writeInt(center_vectors.length);
            for(int i = 0; i < center_vectors.length; i++) {
            	out.writeInt(i);
            	center_vectors[i].write(out);
            }
        }

        @Override
        public void readFields(DataInput dataInput) throws IOException, EOFException{
        	int kClusters = dataInput.readInt();
        	center_vectors = new VectorWritable[kClusters];
            for(int i = 0; i < kClusters; i++) {
            	int k = dataInput.readInt();
            	center_vectors[k] = new VectorWritable();
            	center_vectors[k].readFields(dataInput);
            }
        }
        
        public int findNearestCenter(VectorWritable other_vector) {
        	double nearest = Double.MAX_VALUE;
        	int index = -1;
        	
        	for(int i = 0; i < center_vectors.length; i++) {
        		double dist = VectorMetrics.euclideanDistance(center_vectors[i].getVector(), other_vector.getVector());
        		if(dist < nearest) {
        			nearest = dist;
        			index = i;
        		}
        	}
        	
        	return index;
        }
        
        public double min_similartiy(ClusterCentersWritable other_centers) {
        	double min_similarity = Double.MAX_VALUE;
        	for(int i = 0; i < center_vectors.length; i++) {
        		double cosine_similarity = VectorMetrics.cosineSimilarity(center_vectors[i].getVector(), other_centers.center_vectors[i].getVector());
        		min_similarity = min_similarity < cosine_similarity ? min_similarity : cosine_similarity;
        	}
        	return min_similarity;
        }
        
        public String toString() {
			String result = "";
			for(int i = 0; i < center_vectors.length; i++) {
				result += "Partition " + i + ": " + center_vectors[i].toString() + "\n";
			}
			return result;
        }
}