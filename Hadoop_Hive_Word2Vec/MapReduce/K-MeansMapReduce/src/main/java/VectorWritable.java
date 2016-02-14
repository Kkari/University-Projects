import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

import org.apache.hadoop.io.Writable;

public class VectorWritable implements Writable {

        private float[] vector;

        public VectorWritable(float[] vector) {
            this.vector = vector;
        }
        
        public VectorWritable() {
        }

        public VectorWritable(VectorWritable other_vector) {
        	this.vector = new float[other_vector.vector.length];
        	for(int i = 0; i < vector.length; i++) {
        		this.vector[i] = other_vector.vector[i];
        	}
		}

		public float[] getVector() {
            return vector;
        }

        public void setVector(float[] vector) {
            this.vector = vector;
        }

        @Override
        public void write(DataOutput out) throws IOException {
            out.writeInt(vector.length);
            for (float val : vector) {
                out.writeFloat(val);
            }
        }

        @Override
        public void readFields(DataInput in) throws IOException {
        	vector = new float[in.readInt()];
            for (int i = 0; i < vector.length; i++) {
                vector[i] = in.readFloat();
            }
        }
        
        public double length() {
        	double length = 0;
        	for(float val : vector) {
        		length += val*val;
        	}
        	return Math.sqrt(length);
        }

        public String toString() {
        	String result = "";
            for (int i = 0; i < vector.length-1; i++) {
                result += Float.toString(vector[i]) + ",";
            }
            result += Float.toString(vector[vector.length-1]);
            return result;
        }
 }