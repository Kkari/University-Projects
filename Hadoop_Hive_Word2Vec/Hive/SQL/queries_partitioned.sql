SELECT words2.word, cosineSimilarity(words.vector, words2.vector) as dist FROM words 
JOIN words as words2 ON words.word = 'germany' AND words.id = words2.id
ORDER BY dist DESC LIMIT 50;