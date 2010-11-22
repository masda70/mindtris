using System;
using System.Collections.Generic;
using System.Text;

namespace ZenithMFramework
{
    public class ConsoleMenu
    {
        string _caption;
        string[] _entries;
        int _nentries;

        string _cursor;
        bool _displayNumbers;

        //Constructeurs
        public ConsoleMenu(string caption, params string[] entries)
        {
            _caption = caption;
            _nentries = entries.Length;
            _entries = new string[_nentries];
            entries.CopyTo(_entries, 0);
            _cursor = "->";
            _displayNumbers = true;
        }
        public ConsoleMenu(string caption, bool displayNumbers, params string[] entries)
        {
            _caption = caption;
            _nentries = entries.Length;
            _entries = new string[_nentries];
            entries.CopyTo(_entries, 0);
            _cursor = "->";
            _displayNumbers = displayNumbers;
        }
        public ConsoleMenu(string caption, string cursor, bool displayNumbers, params string[] entries)
        {
            _caption = caption;
            _nentries = entries.Length;
            _entries = new string[_nentries];
            entries.CopyTo(_entries, 0);
            _cursor = cursor;
            _displayNumbers = displayNumbers;
        }

        //Affichage
        public int Show()
        {
            //Cette fonction renvoie l'indice du choix de l'utilisateur
            //Affichage
            Console.CursorVisible = false;
            Console.WriteLine(_caption);
            Console.WriteLine();
            int[] tops;
            string espace = "".PadLeft(_cursor.Length + 1, ' ');
            for (int i = 0; i < _nentries; i++)
            {
                Console.Write(espace);
                if (_displayNumbers)
                {
                    Console.Write(i);
                    Console.Write(" : ");
                }
                Console.WriteLine(_entries[i]);
            }
            int pos_final = Console.CursorTop;
            tops = Tops();
            ConsoleKeyInfo cki = new ConsoleKeyInfo('a', ConsoleKey.A, false, false, false);
            int current = 0;
            int prec = 0;
            while (cki.Key != ConsoleKey.Enter)
            {
                Console.CursorLeft = 0;
                Console.CursorTop = tops[current];
                Console.Write(_cursor);
                cki = Console.ReadKey();
                if (cki.Key == ConsoleKey.UpArrow)
                {
                    prec = current;
                    current = (current - 1) % _nentries;
                    if (current < 0) current += _nentries;
                }
                if (cki.Key == ConsoleKey.DownArrow)
                {
                    prec = current;
                    current = (current + 1) % _nentries;
                }
                Console.CursorLeft = 0;
                Console.CursorTop = tops[prec];
                Console.Write(espace);
            }
            Console.CursorTop = pos_final;
            Console.CursorLeft = 0;
            Console.CursorVisible = true;
            return current;
        }
        private int[] Tops()
        {
            int[] resultat = new int[_nentries];
            int pos_current = Console.CursorTop;
            int decalage;
            int espace = "".PadLeft(_cursor.Length + 1, ' ').Length;
            int quotient;
            for (int i = _nentries - 1; i > -1; i--)
            {
                decalage = espace + _entries[i].Length;
                decalage += _displayNumbers ? i.ToString().Length + 3 : 0;
                quotient = decalage / Console.BufferWidth;
                if (quotient * Console.BufferWidth == decalage)
                {
                    decalage = quotient;
                }
                else decalage = quotient + 1;
                pos_current -= decalage;
                resultat[i] = pos_current;
            }
            return resultat;

        }

        //Opérations sur les entrées
        public void Insert(string entry, int index)
        {
            //Insère la nouvelle entrée à la position [index], ou à la fin si celle-ci n'est pas valide
            _nentries++;
            string[] nouveau = new string[_nentries];
            int i = 0;
            bool Inserted = false;
            while (i < _nentries)
            {
                if (i == index)
                {
                    nouveau[i] = entry;
                    Inserted = true;
                }
                else
                {
                    nouveau[i] = Inserted ? _entries[i - 1] : _entries[i];
                }
                i++;
            }
            if (!Inserted) nouveau[_nentries - 1] = entry;
            _entries = nouveau;
        }
        public void Sort()
        {
            //Tri les entrées dans l'ordre alphabétique
            //NB : En attendant le Quicksort, le tri du framework .NET est utilisé
            Array.Sort(_entries);
        }

        //Accès au champs en lecture
        public string Caption
        {
            get { return _caption; }
        }
        public string[] Entries
        {
            get { return _entries; }
        }
        public int NEntries
        {
            get { return _nentries; }
        }
        public string Cursor
        {
            get { return _cursor; }
        }
        public bool DisplayNumbers
        {
            get { return _displayNumbers; }
        }
    }
}
