import java.io.IOException;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.mapreduce.Reducer;

public class ReducerKMeans extends Reducer<IntWritable, WordVectorWritable, IntWritable, VectorWritable> {

	private Configuration conf;
	private int vectorLength;

    @Override
    public void setup(Context context) throws IOException, InterruptedException {
    	super.setup(context);
   
    	conf = context.getConfiguration();
    	vectorLength = conf.getInt("vectorLength", 200);
    }
    
    @Override
    public void reduce(IntWritable key, Iterable<WordVectorWritable> values, Context context) throws IOException, InterruptedException {
    	double[] sum = new double[vectorLength];
    	float[] mean = new float[vectorLength];
    	int num = 0;
    	
    	// calculate sum of vectors
    	for(WordVectorWritable wv : values) {
    		for(int i = 0; i < vectorLength; i++) {
    			sum[i] +=  wv.getVector().getVector()[i];
    		}
    		num++;
    	}
    	
    	// calculate mean of vectors (new center vector)
    	for(int i = 0; i < vectorLength; i++) {
    		mean[i] = (float)(sum[i]/num);
    	}
    	
    	System.out.println("========== Partition " + key+ ": " + num + " Vectors ==========");
    	context.write(key, new VectorWritable(mean));
    }
}
