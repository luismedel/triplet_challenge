
using System.Text.RegularExpressions;

Dictionary<string, int> counts = new Dictionary<string, int>(StringComparer.InvariantCultureIgnoreCase);

void inc(string key) => counts[key] = counts.TryGetValue(key, out int count) ? count + 1 : 1;

var input = args.Length != 0 ? args[0] : throw new ArgumentException("Input file expected");
var words = Regex.Split(File.ReadAllText(input), @"[^A-Za-z']+", RegexOptions.Compiled);

if (words.Length < 3)
    throw new InvalidDataException("Too few words");

for (int i = 2; i < words.Length; i++)
    inc($"{words[i - 2]} {words[i - 1]} {words[i]}");

var output = counts.OrderByDescending(kv => kv.Value).Take(3).Select(kv => $"{kv.Key} - {kv.Value}");

Console.WriteLine(String.Join("\n", output));
