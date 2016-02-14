SELECT words2.word, cosineSimilarity(" + table + ".vector, words2.vector) " +
					"as dist FROM " + table + " " + 
					"JOIN " + table + " as words2 ON " + table + ".word = '" + word + "' AND " + table + ".id = words2.id " + 
					"ORDER BY dist DESC LIMIT " + LIMIT
