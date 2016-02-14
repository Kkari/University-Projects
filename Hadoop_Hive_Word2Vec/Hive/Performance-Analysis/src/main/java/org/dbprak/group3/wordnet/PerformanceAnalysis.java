package org.dbprak.group3.wordnet;

import java.awt.FlowLayout;
import java.awt.Font;
import java.util.ArrayList;

import javax.swing.JFrame;

import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.CategoryAxis;
import org.jfree.chart.axis.NumberAxis;
import org.jfree.chart.labels.BoxAndWhiskerToolTipGenerator;
import org.jfree.chart.plot.CategoryPlot;
import org.jfree.chart.renderer.category.BoxAndWhiskerRenderer;
import org.jfree.data.statistics.DefaultBoxAndWhiskerCategoryDataset;
import org.jfree.ui.ApplicationFrame;
import org.jfree.ui.RefineryUtilities;

import net.sf.extjwnl.JWNLException;
import net.sf.extjwnl.data.IndexWord;
import net.sf.extjwnl.data.POS;
import net.sf.extjwnl.dictionary.Dictionary;

public class PerformanceAnalysis extends ApplicationFrame {
	
	public PerformanceAnalysis(String title) {
		super(title);
		 ArrayList<String> tables = new ArrayList<String>();
	        tables.add("words");
	        tables.add("words_partitioned ");
	        
			int num_words = 5;
			final DefaultBoxAndWhiskerCategoryDataset datasetBox= new DefaultBoxAndWhiskerCategoryDataset();
		     
	        for(String table: tables){
				System.out.println("gestartet");
				
				//ArrayList<String> queryWords = getRandomWords(num_words);
				ArrayList<String> queryWords = new ArrayList<String>();
				queryWords.add("soft");
				queryWords.add("easy");
				queryWords.add("hard");
				queryWords.add("sexy");
				queryWords.add("cute");
				queryWords.add("classic");
				
				String word;

				ArrayList<Long> timeQuery1 = new ArrayList<Long>();
				for(int i = 0; i < num_words; i++) {
					//System.out.print((i+1) + ". Run Query for '" + queryWords.get(i) + "' ...");
					word =  queryWords.get(i);
					String query1 = "SELECT words2.word, cosineSimilarity(" + table + ".vector, words2.vector) " +
							"as dist FROM " + table + " " + 
							"JOIN " + table + " as words2 ON " + table + ".word = '" + word + "' AND " + table + ".id = words2.id " + 
							"ORDER BY dist DESC LIMIT 100";
					PerformanceQuery pquery = new PerformanceQuery(query1);
					if (pquery.getResultSize() != 0) {
						timeQuery1.add(pquery.getEstimatedTime());
						System.out.println(pquery.getEstimatedTime());
					} else {
						System.out.println(" no results!");
					}
				}
				datasetBox.add(timeQuery1, "Runtime ", "Q1 " + table);
				long sum = 0;
				for(long l:timeQuery1)
				{
					sum += l;
				}
				System.out.println("Query 1 Average:" + sum/timeQuery1.size());
				
				ArrayList<Long> timeQuery2 = new ArrayList<Long>();
				for(int i = 0; i < num_words; i++) {
					//System.out.print((i+1) + ". Run Query for '" + queryWords.get(i) + "' ...");
					word =  queryWords.get(i);
					String word1 = queryWords.get(num_words-i);
					String query2 = "SELECT cosineSimilarity(words.vector, words2.vector) as dist FROM words "
							+ "CROSS JOIN (SELECT * FROM words WHERE word='"+word1+"') AS words2 "
							+ "WHERE words.word ='"+word+"'";
					PerformanceQuery pquery = new PerformanceQuery(query2);
					if (pquery.getResultSize() != 0) {
						timeQuery2.add(pquery.getEstimatedTime());
						System.out.println(pquery.getEstimatedTime());
					} else {
						System.out.println(" no results!");
					}
				}
				datasetBox.add(timeQuery2, "Runtime ", "Q2: " + table);
				sum = 0;
				for(long l:timeQuery2)
				{
					sum += l;
				}
				System.out.println("Query 2 Average:" + sum/timeQuery2.size());
				
				ArrayList<Long> timeQuery3 = new ArrayList<Long>();
				for(int i = 0; i < num_words; i++) {
					//System.out.print((i+1) + ". Run Query for '" + queryWords.get(i) + "' ...");
					word =  queryWords.get(i);
					String query3 = "SELECT words2.word, cosineSimilarity(" + table + ".vector, words2.vector) " +
							"as dist FROM " + table + " " + 
							"JOIN " + table + " as words2 ON " + table + ".word LIKE '%" + word + "%' AND " + table + ".id = words2.id " + 
							"ORDER BY dist DESC LIMIT 100";
					PerformanceQuery pquery = new PerformanceQuery(query3);
					if (pquery.getResultSize() != 0) {
						timeQuery3.add(pquery.getEstimatedTime());
						System.out.println(pquery.getEstimatedTime());
					} else {
						System.out.println(" no results!");
					}
				}
				datasetBox.add(timeQuery3, "Runtime ", "Q3: " + table);
				sum = 0;
				for(long l:timeQuery3)
				{
					sum += l;
				}
				System.out.println("Query 3 Average:" + sum/timeQuery3.size());
	        }
			HiveConnection.closeConnection();
			
			
	        final CategoryAxis xAxis = new CategoryAxis("Querys");
	        final NumberAxis yAxis = new NumberAxis("Runtime ms");
	        yAxis.setAutoRangeIncludesZero(false);
	        final BoxAndWhiskerRenderer renderer = new BoxAndWhiskerRenderer();
	        renderer.setFillBox(false);
	        renderer.setToolTipGenerator(new BoxAndWhiskerToolTipGenerator());
	        final CategoryPlot plot = new CategoryPlot(datasetBox, xAxis, yAxis, renderer);

	        final JFreeChart boxChart = new JFreeChart(
	                "Statistik Ausführungszeit der Querys in java gemessen",
	                new Font("SansSerif", Font.BOLD, 14),
	                plot,
	                true
	        );
	        ChartPanel boxChartPanel = new ChartPanel(boxChart);
			 JFrame frame = new JFrame("Charts");
		     frame.setLayout(new FlowLayout());
		     frame.setSize(800, 800);
		     frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		     
	        frame.getContentPane().add(boxChartPanel);
	        frame.setVisible(true);
	}

	public static void main(String args[]) {
		// disable verbose logging
        Logger rootLogger = Logger.getRootLogger();
        rootLogger.setLevel(Level.ERROR);
        
        final PerformanceAnalysis demo = new PerformanceAnalysis("Hive Performance Box-and-Whisker Chart");
        demo.pack();
        RefineryUtilities.centerFrameOnScreen(demo);
        demo.setVisible(true);

       
	}

	private static ArrayList<String> getRandomWords(int num_words) {
		Dictionary dictionary = null;
		ArrayList<String> queryWords = new ArrayList<String>();
		try {
			
			dictionary = Dictionary.getDefaultResourceInstance();
		    for(int i = 0; i < num_words ;i++){
		    	IndexWord random_adjective = dictionary.getRandomIndexWord(POS.ADJECTIVE);
		    	queryWords.add(random_adjective.getLemma());
		    }
		    
		} catch (JWNLException e) {
			System.out.println("JWNL Error: " + e.getMessage());
			e.printStackTrace();
		}
		return queryWords;
	}
}