import java.io.IOException;
import java.util.Locale;
import java.util.Scanner;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Mapper;

public class MapperKMeans extends Mapper<Object, Text, IntWritable, WordVectorWritable>  {
    
	private Configuration conf;
	private int vectorLength;
	
	ClusterCentersWritable currentCenters;

    @Override
    public void setup(Context context) throws IOException, InterruptedException {
    	super.setup(context);
   
    	conf = context.getConfiguration();
    	vectorLength = conf.getInt("vectorLength", 200);
    	currentCenters = VectorKMeans.readCentersFromFile(conf); 
    }
    
    @Override
    public void map(Object key, Text value, Context context) throws IOException, InterruptedException {
    	// read raw input with scanner
        Scanner scanner = new Scanner(value.toString());
        scanner.useLocale(Locale.US);
        scanner.useDelimiter(":|,");
        
        // read word
        String word = scanner.next();
        // read float
        float[] vector = new float[vectorLength];
        for(int i = 0; i < vectorLength; i++) {
        	vector[i] = scanner.nextFloat();
        }
        
        VectorWritable vectorWritable = new VectorWritable(vector);
        
        // assign pseudorandom partition
        IntWritable partition = new IntWritable(currentCenters.findNearestCenter(vectorWritable));
        context.write(partition, new WordVectorWritable(word, vectorWritable));
    }
}
