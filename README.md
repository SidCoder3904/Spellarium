# Spellarium

**Spellarium** is a high-performance spell-checking and word suggestion tool implemented in C. The project utilizes tries and Bloom filters to efficiently handle word lookups and suggestions, making it well-suited for large-scale text processing.

## Features

- **Trie-Based Spell Checking**: Uses a trie data structure for precise and fast word retrieval.
- **Bloom Filter**: Implements a Bloom filter for quick checks on word existence.
- **Performance Optimization**: Trained on a dictionary of over 3.7 million words, with multithreading support for handling large inputs efficiently.
- **Analysis and Comparison**: Includes modes to compare the performance of tries and Bloom filters, providing insights into their speed and accuracy.

## Usage

1. **Spell Checking and Autocorrect:**

   - Select mode 1 from the menu to enter spell-checking and autocorrect mode.
   - Input a sentence or type `exit` to leave.
   - The tool will display suggestions for any misspelled words.

2. **Comparison Mode:**

   - Select mode 2 to compare the performance of tries and Bloom filters.
   - Enter a word or type `exit` to leave.
   - The tool will show which data structure contains the word and provide performance metrics.

3. **Optimization Mode:**

   - Select mode 3 for future enhancements and optimization experiments (not yet implemented).

4. **Quit:**

   - Select mode 4 to exit the application.

## Performance

The tool is designed for optimal performance with features including:

- **Multithreading**: Improves processing speed.
- **Extensive Dictionary**: Over 3.7 million words.
- **Efficiency Analysis**: Performance comparison between tries and Bloom filters.

## Acknowledgments

- [Levenshtein Distance](https://en.wikipedia.org/wiki/Levenshtein_distance)
- [Jaro-Winkler Distance](https://en.wikipedia.org/wiki/Jaro–Winkler_distance)
- [Bloom Filter](https://en.wikipedia.org/wiki/Bloom_filter)

For any questions or feedback, feel free to open an issue on GitHub.

