package org.dbprak.group3.util;


import java.util.Iterator;
import java.util.List;

import org.apache.hadoop.io.FloatWritable;

public class VectorMetrics {

    public static Double cosineDistance(List<FloatWritable> v1, List<FloatWritable> v2) {
        double sum_ab = 0;
        double sum_a_squared = 0;
        double sum_b_squared = 0;
        
        Iterator<FloatWritable> v1_it = v1.iterator();
        Iterator<FloatWritable> v2_it = v2.iterator();
        
        while(v1_it.hasNext()) {
            Float a = v1_it.next().get();
            Float b = v2_it.next().get();
            sum_ab += a*b;
            sum_a_squared += a*a;
            sum_b_squared += b*b;
        }
        
        double similarity = sum_ab / (Math.sqrt(sum_a_squared) * Math.sqrt(sum_b_squared)); 
        return similarity;
    }
    
    public static Double euclideanDistance(List<FloatWritable> v1, List<FloatWritable> v2) {
        double sum = 0;
        
        Iterator<FloatWritable> v1_it = v1.iterator();
        Iterator<FloatWritable> v2_it = v2.iterator();
        
        while(v1_it.hasNext()) {
            Float a = v1_it.next().get();
            Float b = v2_it.next().get();
            sum += Math.pow((a - b), 2);
        }
        
        double similarity = Math.sqrt(sum); 
        return similarity;
    }
    
}
