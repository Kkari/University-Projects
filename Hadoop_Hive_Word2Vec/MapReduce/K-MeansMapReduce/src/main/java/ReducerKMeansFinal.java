import java.io.IOException;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.output.MultipleOutputs;

public class ReducerKMeansFinal extends Reducer<IntWritable, WordVectorWritable, Object, WordVectorWritable> {

	private MultipleOutputs<Object, WordVectorWritable> mout;
	private Configuration conf;
	
    @Override
    public void setup(Context context) throws IOException, InterruptedException {
    	super.setup(context);
    	mout = new MultipleOutputs<Object, WordVectorWritable>(context);
    	conf = context.getConfiguration();
    }
    
    @Override
    public void reduce(IntWritable key, Iterable<WordVectorWritable> values, Context context) throws IOException, InterruptedException {
		String partitionOutputPath = conf.get("partitionsPath") + "/id=" + Integer.toString(key.get())+"/partition.txt";

    	for(WordVectorWritable wv : values) {
    		mout.write((Object)null, wv, partitionOutputPath);
    	}
    }
}
