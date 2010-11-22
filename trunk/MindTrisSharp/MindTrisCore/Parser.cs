using System;
using System.Collections.Generic;
using System.Text;
using System.Net;

namespace ZenithMFramework
{
    public class WebParser
    {
        string _page;
        int _pos;

        Dictionary<string, int> _checkpoints;
        Dictionary<int, int> _checkpoints2;

        //Constructeurs
        public WebParser(string page)
        {
            _page = page;
            _WebParser();
        }
        void _WebParser()
        {
            _pos = 0;
            _checkpoints = new Dictionary<string, int>();
            _checkpoints2 = new Dictionary<int, int>();
        }

        /// <summary>
        /// Set the Position to the index of the last sub-string in the params, and returns true, if found in the page. If not found, returns false.
        /// </summary>
        /// <param name="intermediate_strings">Array of strings to jump successively to before finally stop.</param>
        /// <returns>True if succeeded, false otherwise</returns>
        public bool JumpTo(int startindex, int maxcount, params string[] intermediate_strings)
        {
            //Backup un peu ^^
            int old_pos = _pos;
            _pos = startindex;
            int max = Math.Min(_page.Length, maxcount + startindex);
            try
            {
                for (int i = 0; i < intermediate_strings.Length; i++)
                {
                    _pos = _page.IndexOf(intermediate_strings[i], _pos);
                    if (_pos < 0) throw new IndexOutOfRangeException("not found");
                    if (i < intermediate_strings.Length - 1 && intermediate_strings[i + 1] == intermediate_strings[i]) _pos++;
                    if (_pos > max - 1) { _pos = old_pos; return false; }
                }
                return true;
            }
            catch
            {
                _pos = old_pos;
                return false;
            }
        }
        public bool JumpTo(int maxcount, params string[] intermediate_strings)
        {
            return JumpTo(_pos, maxcount, intermediate_strings);
        }
        public bool JumpTo(params string[] intermediate_strings)
        {
            return JumpTo(_pos, _page.Length - _pos, intermediate_strings);
        }
        public bool JumpAfter(int startindex, int maxcount, params string[] intermediate_strings)
        {
            if (JumpTo(startindex, maxcount, intermediate_strings))
            {
                _pos += intermediate_strings[intermediate_strings.Length - 1].Length;
                return true;
            }
            else return false;
        }
        public bool JumpAfter(int maxcount, params string[] intermediate_strings)
        {
            return JumpAfter(_pos, maxcount, intermediate_strings);
        }
        public bool JumpAfter(params string[] intermediate_strings)
        {
            return JumpAfter(_pos, _page.Length - _pos, intermediate_strings);
        }

        public string JumpToFirstOf(int startindex, int maxcount, params string[] str)
        {
            //Backup un peu ^^
            int old_pos = _pos;
            _pos = startindex;
            int max = Math.Min(_page.Length - startindex, maxcount);
            string result = String.Empty;
            int min = Int32.MaxValue;
            int temp;
            foreach (string s in str)
            {
                temp = _page.IndexOf(s, _pos, max);
                if (temp > -1 && temp < min)
                {
                    min = temp;
                    result = s;
                }
            }
            if (min < Int32.MaxValue)
            {
                _pos = min;
                return result;
            }
            _pos = old_pos;
            return String.Empty;
        }
        public string JumpToFirstOf(int maxcount, params string[] str)
        {
            return JumpToFirstOf(_pos, maxcount, str);
        }
        public string JumpToFirstOf(params string[] str)
        {
            return JumpToFirstOf(_pos, _page.Length - _pos, str);
        }

        /// <summary>
        /// Get the string between two given strings and output it in str. Position in the Page is updated.
        /// </summary>
        /// <param name="first">First delimiter string.</param>
        /// <param name="second">Second delimiter string.</param>
        /// <param name="str">Output string.</param>
        /// <returns>True if succeeded, false otherwise.</returns>
        public bool GetStringBetween(string first, string second, out string str)
        {
            int old_pos = _pos;
            str = "";
            try
            {
                _pos = _page.IndexOf(first, _pos);
                if (_pos >= 0)
                {
                    int next = _page.IndexOf(second, _pos + first.Length);
                    str = _page.Substring(_pos + first.Length, next - _pos - first.Length);
                    _pos += first.Length + str.Length;
                    return true;
                }
                else return false;
            }
            catch
            {
                _pos = old_pos;
                return false;
            }
        }
        /// <summary>
        /// Get the string between two given characters and output it in str. Position in the Page is updated.
        /// </summary>
        /// <param name="first">First delimiter char.</param>
        /// <param name="second">Second delimiter char.</param>
        /// <param name="str">Output string.</param>
        /// <returns>True if succeeded, false otherwise.</returns>
        public bool GetStringBetween(char first, char second, out string str)
        {
            return GetStringBetween(first.ToString(), second.ToString(), out str);
        }

        public string GetStringBetween(char first, char second)
        {
            string result;
            if (GetStringBetween(first, second, out result)) return result;
            return String.Empty;
        }
        public string GetStringBetween(string first, string second)
        {
            string result;
            if (GetStringBetween(first, second, out result)) return result;
            return String.Empty;
        }

        public bool GetStringBefore(string s, out string str)
        {
            int old_pos = _pos;
            try
            {
                int next = _page.IndexOf(s, _pos);
                str = _page.Substring(_pos, next - _pos);
                _pos += str.Length;
                return true;
            }
            catch
            {
                _pos = old_pos;
                str = "";
                return false;
            }
        }
        public bool GetStringBefore(char c, out string str)
        {
            return GetStringBefore(c.ToString(), out str);
        }
        /// <summary>
        /// For a given params strings array of length n, jumps after the n - 1 firsts, and output in str the complete string before the n-nth, not included it.
        /// </summary>
        /// <param name="str">The result string.</param>
        /// <param name="intermediate_strings">Array of intermediate strings to be looked for, the last being the stop mark for the result string. If null or empty, an error is raised.</param>
        /// <returns>Returns true if succeeded, false otherwise</returns>
        public bool GetStringBeforeThroughCheckpoints(out string str, params string[] intermediate_strings)
        {
            if (intermediate_strings != null && intermediate_strings.Length > 0)
            {
                str = "";
                //Recopie des n - 1 premières chaînes
                string[] param = new string[intermediate_strings.Length - 1];
                for (int i = 0; i < intermediate_strings.Length - 1; i++) param[i] = intermediate_strings[i];
                bool succ = JumpAfter(param);
                return succ && GetStringBefore(intermediate_strings[intermediate_strings.Length - 1], out str);
            }
            else throw new ArgumentNullException("intermediate_strings", "Either empty or null. Needs at least 1 stop mark.");
        }

        /// <summary>
        /// Set a checkpoint at the current position in the page.
        /// </summary>
        /// <param name="key">A kind of flagouze to retrieve the checkpoint.</param>
        public void SetCheckpoint(string key)
        {
            SetCheckpoint(key, _pos);
        }
        /// <summary>
        /// Set a checkpoint at a given position in the page. Returns true if the given position is valid, false otherwise.
        /// </summary>
        /// <param name="key">A kind of flagouze to retrieve the checkpoint.</param>
        /// <param name="position">A position.</param>
        /// <returns>True if the given position is valid, false otherwise.</returns>
        public bool SetCheckpoint(string key, int position)
        {
            if (position >= 0 && position < _page.Length)
            {
                _checkpoints[key] = position;
                return true;
            }
            return false;
        }
        /// <summary>
        /// Get the position of the checkpoint of a given key. Returns the position, or -1 if the given key isn't found.
        /// </summary>
        /// <param name="key">A key.</param>
        /// <returns>Returns the position, or -1 if the given key isn't found.</returns>
        public int GetCheckpoint(string key)
        {
            int res;
            if (_checkpoints.TryGetValue(key, out res)) return res;
            return -1;
        }

        public bool SetCheckpoint(int key, int position)
        {
            if (position >= 0 && position < _page.Length)
            {
                _checkpoints2[key] = position;
                return true;
            }
            return false;
        }
        public void SetCheckpoint(int key)
        {
            SetCheckpoint(key, _pos);
        }
        public int GetCheckpoint(int key)
        {
            int res;
            if (_checkpoints2.TryGetValue(key, out res)) return res;
            return -1;
        }

        public int this[string key]
        {
            get
            {
                return _checkpoints[key];
            }
            set
            {
                if (value >= 0 && value < _page.Length)
                {
                    _checkpoints[key] = value;
                }
            }
        }
        public int this[int key]
        {
            get
            {
                return _checkpoints2[key];
            }
            set
            {
                if (value >= 0 && value < _page.Length)
                {
                    _checkpoints2[key] = value;
                }
            }
        }

        /// <summary>
        /// The entire HTML code of the page. This property is read-only.
        /// </summary>
        public string Page
        {
            get { return _page; }
        }
        /// <summary>
        /// Set or get the position in the HTML code.
        /// </summary>
        public int Position
        {
            set { _pos = value; }
            get { return _pos; }
        }
        /// <summary>
        /// A collection of all the checkpoints set in the page.
        /// </summary>
        public Dictionary<string, int> Checkpoints
        {
            get { return _checkpoints; }
        }
        public Dictionary<int, int> Checkpoints2
        {
            get { return _checkpoints2; }
        }

    }
}
