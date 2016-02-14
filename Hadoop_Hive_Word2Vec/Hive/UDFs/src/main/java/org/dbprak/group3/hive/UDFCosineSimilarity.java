package org.dbprak.group3.hive;

import java.util.List;

import org.apache.hadoop.hive.ql.exec.UDFArgumentException;
import org.apache.hadoop.hive.ql.exec.UDFArgumentLengthException;
import org.apache.hadoop.hive.ql.metadata.HiveException;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDF;
import org.apache.hadoop.hive.serde2.objectinspector.ObjectInspector;
import org.apache.hadoop.hive.serde2.objectinspector.primitive.FloatObjectInspector;
import org.apache.hadoop.hive.serde2.objectinspector.primitive.PrimitiveObjectInspectorFactory;
import org.apache.hadoop.io.FloatWritable;
import org.dbprak.group3.util.VectorMetrics;
import org.apache.hadoop.hive.serde2.objectinspector.ListObjectInspector;

public class UDFCosineSimilarity extends GenericUDF {
	
	ListObjectInspector v1inspector;
	ListObjectInspector v2inspector;

	@Override
	public String getDisplayString(String[] arg0) {
		return "cosineSimilarity()";
	}

	@Override
	public ObjectInspector initialize(ObjectInspector[] arguments) throws UDFArgumentException {
		// check the number of arguments
	    if (arguments.length != 2) {
	        throw new UDFArgumentLengthException("cosineSimilarity expects 2 arguments: Array<Float>, Array<Float>");
	    }
	    
	    // check if arguments are arrays
	    if(!(arguments[0] instanceof ListObjectInspector) || !(arguments[1] instanceof ListObjectInspector)) {
	    	throw new UDFArgumentException("cosineSimilarity expects 2 arguments: Array<Float>, Array<Float>");
	    }
	    
	    // check if arguments are arrays of floats
	    v1inspector = (ListObjectInspector) arguments[0];
	    v2inspector = (ListObjectInspector) arguments[1];
	    
	    if(!(v1inspector.getListElementObjectInspector() instanceof FloatObjectInspector) 
	    		|| !(v2inspector.getListElementObjectInspector() instanceof FloatObjectInspector)) {
	    	throw new UDFArgumentException("cosineSimilarity expects 2 arguments: Array<Float>, Array<Float>");
	    }
	    
		// declare UDF return type
		return PrimitiveObjectInspectorFactory.javaDoubleObjectInspector;
	}

	@Override
	@SuppressWarnings("unchecked")
	public Object evaluate(DeferredObject[] arguments) throws HiveException {
		
		List<FloatWritable> v1 = (List<FloatWritable>) v1inspector.getList(arguments[0].get());
		List<FloatWritable> v2 = (List<FloatWritable>) v2inspector.getList(arguments[1].get());
		
		// check for null values
		if(v1 == null || v2 == null)
			return null;
		
		// both vectors must have equal size
		if(v1.size() != v2.size()) {
			throw new HiveException("cosineSimilarity: both vectors must have equal size");
		}
		
		return VectorMetrics.cosineDistance(v1, v2);
	}

}
