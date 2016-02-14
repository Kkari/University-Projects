DROP FUNCTION IF EXISTS cosineSimilarity;

CREATE FUNCTION cosineSimilarity AS 'org.dbprak.group3.hive.UDFCosineSimilarity' 
USING JAR 'hdfs:///usr/local/hadoop/dbprak/group3/hive-udfs-1.0.jar';
