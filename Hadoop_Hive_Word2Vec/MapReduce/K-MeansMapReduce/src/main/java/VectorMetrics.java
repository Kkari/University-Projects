
public class VectorMetrics {
        public static double cosineSimilarity(float[] v1, float[] v2) {
            double sum_ab = 0;
            double sum_a_squared = 0;
            double sum_b_squared = 0;

            for(int i = 0; i < v1.length; i++) {
            	double a = (double)v1[i];
                double b = (double)v2[i];
                sum_ab += a * b;
                sum_a_squared += a * a;
                sum_b_squared += b * b;
            }

            double similarity = sum_ab / (Math.sqrt(sum_a_squared) * Math.sqrt(sum_b_squared));
            return similarity;
        }
        
        public static double angularSimilarity(float[] v1, float[] v2) {
        	return (1 - (Math.acos(cosineSimilarity(v1,v2)/Math.PI)));
        }
        
        public static double euclideanDistance(float[] v1, float[] v2) {
            double sum = 0;

            for(int i = 0; i < v1.length; i++) {
                Float a = v1[i];
                Float b = v2[i];
                sum += Math.pow((a - b), 2);
            }

            double similarity = Math.sqrt(sum);
            return similarity;
        }
}