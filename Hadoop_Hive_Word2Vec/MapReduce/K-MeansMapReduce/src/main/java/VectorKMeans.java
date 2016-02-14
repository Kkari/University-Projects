import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.conf.Configured;
import org.apache.hadoop.fs.FileStatus;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.fs.permission.FsPermission;
import org.apache.hadoop.io.*;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.mapreduce.lib.output.MultipleOutputs;
import org.apache.hadoop.mapreduce.lib.output.SequenceFileOutputFormat;
import org.apache.hadoop.mapreduce.lib.output.TextOutputFormat;
import org.apache.hadoop.util.Tool;
import org.apache.hadoop.util.ToolRunner;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;

public class VectorKMeans extends Configured implements Tool {

    @Override
    public int run(String[] args) throws IOException, ClassNotFoundException, InterruptedException {
        Configuration conf = new Configuration();

        // set runtime options
        conf.set("mapred.reduce.java.opts", "-Xmx1024M");

        // set global parameters
        int kClusters = args.length >= 4 ? Integer.parseInt(args[3]) : 16;
        int vectorLength = args.length >= 5 ? Integer.parseInt(args[4]) : 200;
        conf.setInt("kClusters", kClusters);
        conf.setInt("vectorLength", vectorLength);
        
        FileSystem fs = FileSystem.get(conf);

        // paths
        Path inputPath = new Path(args[0]);
        Path outputPath = new Path(args[1]+"/mapreduce");        
        Path centerPath = new Path(args[1]+"/centers");
        Path partitionsPath = new Path(args[2]);
        conf.set("inputPath", inputPath.toString());
        conf.set("outputPath", outputPath.toString());
        conf.set("centerPath", centerPath.toString());
        conf.set("partitionsPath", partitionsPath.toString());

        writeInitialCenters(conf);

        // determine new cluster centers until the solution converges
        int round = 0; boolean convergence = false;
        while(!convergence) {
	        if (fs.exists(outputPath))
	            fs.delete(outputPath, true);
	        
	        round++;
	        ClusterCentersWritable centers_old = readCentersFromFile(conf);
	        
		    Job job = Job.getInstance(conf, "Vector Kmeans Round " + round);
		    System.out.println("========== Vector Kmeans Round " + round + " ==========");
		    job.setJarByClass(VectorKMeans.class);
		        
	        job.setMapperClass(MapperKMeans.class);
	        job.setReducerClass(ReducerKMeans.class);

	        job.setMapOutputKeyClass(IntWritable.class);
	        job.setMapOutputValueClass(WordVectorWritable.class);
	        job.setOutputKeyClass(IntWritable.class);
	        job.setOutputValueClass(VectorWritable.class);
	        job.setOutputFormatClass(SequenceFileOutputFormat.class);
	        
	        FileInputFormat.addInputPath(job, inputPath);
	        FileOutputFormat.setOutputPath(job, outputPath);
	        
	        job.waitForCompletion(true);
	        
	        ClusterCentersWritable centers_new = readCentersFromOutput(conf);
	        
	        // stop partitioning when the minimal cosine similarity of the center
	        // vectors is greater 0.98, i.e. they didn't change much
	        double min_similarity = centers_old.min_similartiy(centers_new);
	        convergence = min_similarity > 0.98;
	        
	        writeCentersToFile(conf, centers_new);
	        
	        // print some statistics of that round
	        System.out.println("========== Min Similarity: " + min_similarity + "==========");
         }    
         System.out.println("========== Partitioning Finished: " + round + " Rounds Required! ==========");
         
         // a last mapreduce job to split the original data into partition files
         Job job = Job.getInstance(conf, "Vector KMeans Write Partitions");
         System.out.println("========== Vector Kmeans Write Partitions ==========");
         
		 job.setJarByClass(VectorKMeans.class);
		 job.setMapperClass(MapperKMeans.class);
		 job.setReducerClass(ReducerKMeansFinal.class);
		 
	     job.setMapOutputKeyClass(IntWritable.class);
	     job.setMapOutputValueClass(WordVectorWritable.class);
	     job.setOutputKeyClass(Object.class);
	     job.setOutputValueClass(WordVectorWritable.class);
		 
	     // delete tmp output
		 fs.delete(outputPath, true);
		 // create folder for partitions
		 if(fs.exists(partitionsPath))
			 fs.delete(partitionsPath, true);
		 fs.mkdirs(partitionsPath, FsPermission.createImmutable((short)0775));
		 
		 // multiple outputs for partition files
		 MultipleOutputs.addNamedOutput(job, "partitions", TextOutputFormat.class, Object.class, WordVectorWritable.class);
		 FileInputFormat.addInputPath(job, inputPath);
		 FileOutputFormat.setOutputPath(job, outputPath);
		 
		 // distribute vectors to partitions
		 job.waitForCompletion(true);
		 
		 // delete tmp output
		 fs.delete(outputPath, true);

         return 1;
    }
    
    // helper function to initialize ClusterCentersWritable datastructure from centers file
    public static ClusterCentersWritable readCentersFromFile(Configuration conf) throws IOException {
        FileSystem fs = FileSystem.get(conf);
        Path centerPath = new Path(conf.get("centerPath"));
		DataInputStream centerInputFile = fs.open(centerPath);
		ClusterCentersWritable centers = new ClusterCentersWritable();
		centers.readFields(centerInputFile);
		centerInputFile.close();
		return centers;
    }
    
    // helper function to write ClusterCentersWritable datastructure to centers file
    public static void writeCentersToFile(Configuration conf, ClusterCentersWritable centers) throws IOException {
        FileSystem fs = FileSystem.get(conf);
        Path centerPath = new Path(conf.get("centerPath"));
		DataOutputStream centerOutputFile = fs.create(centerPath, true);
		centers.write(centerOutputFile);
		centerOutputFile.close();
    }
    
    // helper function to read centers after mapreduce round and write them to centers file
    public static ClusterCentersWritable readCentersFromOutput(Configuration conf) throws IOException {
    	FileSystem fs = FileSystem.get(conf);
        FileStatus[] fss = fs.listStatus(new Path(conf.get("outputPath")));
        ClusterCentersWritable centers = new ClusterCentersWritable(conf.getInt("kClusters", 100));
        
        for (FileStatus status : fss) {
            Path path = status.getPath();
            // only read the part-r-* files
            if(!path.getName().equals("_SUCCESS")) {
            	
                SequenceFile.Reader reader = new SequenceFile.Reader(conf, SequenceFile.Reader.file(path));
                IntWritable key = new IntWritable();
                VectorWritable value = new VectorWritable();
                while (reader.next(key,value)) {
                    centers.setCenter(key.get(), new VectorWritable(value));
                }
                reader.close();
            }
        }
    	return centers;
    }
    
    // helper function to create initial centers with random values and write the datastructure to centers file
    private static void writeInitialCenters(Configuration conf) throws IOException {
		
		ClusterCentersWritable initialCenters = 
				new ClusterCentersWritable(conf.getInt("kClusters", -1), conf.getInt("vectorLength", -1), -1.5F, 1.5F);
		writeCentersToFile(conf, initialCenters);
    }

    public static void main(String[] args) {
    	if(args.length >= 3 && args.length <= 5) {
	        int res;
	        
			try {
				res = ToolRunner.run(new Configuration(), new VectorKMeans(), args);
		        System.exit(res);
			} catch (Exception e) {
				System.err.println("An unrecoverable errore occured: " + e.getMessage());
				e.printStackTrace();
			}
			
    	} else {
    		System.out.println("");
    		System.out.println("Usage:");
    		System.out.println("hadoop jar KMeans-1.0.jar <input_path> <tmp_path> <output_path> <partitions>  <vector_length> ");
    		System.out.println("");
    		System.out.println("    <input_path>: folder containing word vector data");
    		System.out.println("    <tmp_path>: folder for intermediate output");
    		System.out.println("    <output_path>: target folder for partioned data");
    		System.out.println("    <partitions>: number of partitions (default 16)");
    		System.out.println("    <vector_length>: size of vectors (default 200)");
    	}
    }

}