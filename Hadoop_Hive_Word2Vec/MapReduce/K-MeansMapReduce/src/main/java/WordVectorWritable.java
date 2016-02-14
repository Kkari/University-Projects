import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

import org.apache.hadoop.io.Writable;

public class WordVectorWritable implements Writable {

    	private String word;
        private VectorWritable vector;

        public WordVectorWritable(String word, VectorWritable vector) {
        	this.word = word;
            this.vector = vector;
        }
        
        public WordVectorWritable() {
        }

        public VectorWritable getVector() {
            return vector;
        }

        public void setVector(VectorWritable vector) {
            this.vector = vector;
        }

        @Override
        public void write(DataOutput out) throws IOException {
            out.writeUTF(word.toString());
            vector.write(out);
        }

        @Override
        public void readFields(DataInput in) throws IOException {
        	word = in.readUTF();
        	vector = new VectorWritable();
        	vector.readFields(in);
        }

        public String getWord() {
            return word;
        }

        public void setWord(String word) {
            this.word = word;
        }

        public String toString() {
            String result = new String("");
            result += this.word.toString() + ":";
            result += this.vector.toString();
            return result;
        }
 }