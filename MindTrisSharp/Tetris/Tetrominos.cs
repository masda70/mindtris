using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;

namespace Tetris
{
    [Flags]
    public enum FieldState { Free = 0, Static = 1 << 0, Dynamic = 1 << 1};

    class Tetrominos
    {
        static public Vector2[, ,] CreateFigures()
        {

            #region Creating figures
            // Figures[figure's number, figure's modification, figure's block number] = Vector2
            // At all figures is 7, every has 4 modifications (for cube all modifications the same)
            // and every figure consists from 4 blocks

            Vector2[, ,] Figures = new Vector2[7, 4, 4];

            int k = 0;
            // I-figures
            for (int i = 0; i < 4; i += 2)
            {
                Figures[k, i, 0] = new Vector2(0, 0);
                Figures[k, i, 1] = new Vector2(1, 0);
                Figures[k, i, 2] = new Vector2(2, 0);
                Figures[k, i, 3] = new Vector2(3, 0);
                Figures[k, i + 1, 0] = new Vector2(1, 0);
                Figures[k, i + 1, 1] = new Vector2(1, 1);
                Figures[k, i + 1, 2] = new Vector2(1, 2);
                Figures[k, i + 1, 3] = new Vector2(1, 3);
            }
            k++;
            // J-figures
            Figures[k, 0, 0] = new Vector2(0, 0);
            Figures[k, 0, 1] = new Vector2(0, 1);
            Figures[k, 0, 2] = new Vector2(1, 1);
            Figures[k, 0, 3] = new Vector2(2, 1);

            Figures[k, 1, 0] = new Vector2(1, 0);
            Figures[k, 1, 1] = new Vector2(2, 0);
            Figures[k, 1, 2] = new Vector2(1, 1);
            Figures[k, 1, 3] = new Vector2(1, 2);

            Figures[k, 2, 0] = new Vector2(0, 0);
            Figures[k, 2, 1] = new Vector2(1, 0);
            Figures[k, 2, 2] = new Vector2(2, 0);
            Figures[k, 2, 3] = new Vector2(2, 1);

            Figures[k, 3, 0] = new Vector2(2, 0);
            Figures[k, 3, 1] = new Vector2(2, 1);
            Figures[k, 3, 2] = new Vector2(1, 2);
            Figures[k, 3, 3] = new Vector2(2, 2);

            k++;

            // L-figures
            Figures[k, 0, 0] = new Vector2(0, 1);
            Figures[k, 0, 1] = new Vector2(1, 1);
            Figures[k, 0, 2] = new Vector2(2, 1);
            Figures[k, 0, 3] = new Vector2(2, 0);

            Figures[k, 1, 0] = new Vector2(1, 0);
            Figures[k, 1, 1] = new Vector2(2, 2);
            Figures[k, 1, 2] = new Vector2(1, 1);
            Figures[k, 1, 3] = new Vector2(1, 2);

            Figures[k, 2, 0] = new Vector2(0, 0);
            Figures[k, 2, 1] = new Vector2(1, 0);
            Figures[k, 2, 2] = new Vector2(2, 0);
            Figures[k, 2, 3] = new Vector2(0, 1);

            Figures[k, 3, 0] = new Vector2(2, 0);
            Figures[k, 3, 1] = new Vector2(2, 1);
            Figures[k, 3, 2] = new Vector2(1, 0);
            Figures[k, 3, 3] = new Vector2(2, 2);

            k++;

            // O-figure
            for (int i = 0; i < 4; i++)
            {
                Figures[k, i, 0] = new Vector2(1, 0);
                Figures[k, i, 1] = new Vector2(2, 0);
                Figures[k, i, 2] = new Vector2(1, 1);
                Figures[k, i, 3] = new Vector2(2, 1);
            }

            k++;

            // S-figures
            for (int i = 0; i < 4; i += 2)
            {
                Figures[k, i, 0] = new Vector2(0, 1);
                Figures[k, i, 1] = new Vector2(1, 1);
                Figures[k, i, 2] = new Vector2(1, 0);
                Figures[k, i, 3] = new Vector2(2, 0);
                Figures[k, i + 1, 0] = new Vector2(1, 0);
                Figures[k, i + 1, 1] = new Vector2(1, 1);
                Figures[k, i + 1, 2] = new Vector2(2, 1);
                Figures[k, i + 1, 3] = new Vector2(2, 2);
            }

            k++;

            // T-figures
            Figures[k, 0, 0] = new Vector2(0, 1);
            Figures[k, 0, 1] = new Vector2(1, 1);
            Figures[k, 0, 2] = new Vector2(2, 1);
            Figures[k, 0, 3] = new Vector2(1, 0);
            Figures[k, 1, 0] = new Vector2(1, 0);
            Figures[k, 1, 1] = new Vector2(1, 1);
            Figures[k, 1, 2] = new Vector2(1, 2);
            Figures[k, 1, 3] = new Vector2(2, 1);
            Figures[k, 2, 0] = new Vector2(0, 0);
            Figures[k, 2, 1] = new Vector2(1, 0);
            Figures[k, 2, 2] = new Vector2(2, 0);
            Figures[k, 2, 3] = new Vector2(1, 1);
            Figures[k, 3, 0] = new Vector2(2, 0);
            Figures[k, 3, 1] = new Vector2(2, 1);
            Figures[k, 3, 2] = new Vector2(2, 2);
            Figures[k, 3, 3] = new Vector2(1, 1);

            k++;

            // Z-figures
            for (int i = 0; i < 4; i += 2)
            {
                Figures[k, i, 0] = new Vector2(0, 0);
                Figures[k, i, 1] = new Vector2(1, 0);
                Figures[k, i, 2] = new Vector2(1, 1);
                Figures[k, i, 3] = new Vector2(2, 1);
                Figures[k, i + 1, 0] = new Vector2(2, 0);
                Figures[k, i + 1, 1] = new Vector2(1, 1);
                Figures[k, i + 1, 2] = new Vector2(2, 1);
                Figures[k, i + 1, 3] = new Vector2(1, 2);
            }

            k++;

            #endregion
            return Figures;
        }

        static public Vector2[, ,] CreateFiguresSRSTrueRotations()
        {

            #region Creating figures
            // Figures[figure's number, figure's modification, figure's block number] = Vector2
            // At all figures is 7, every has 4 modifications (for cube all modifications the same)
            // and every figure consists from 4 blocks

            Vector2[, ,] Figures = new Vector2[7, 4, 4];

            int k = 0;
            // I-figures
            Figures[k, 0, 0] = new Vector2(1, 2);
            Figures[k, 0, 1] = new Vector2(2, 2);
            Figures[k, 0, 2] = new Vector2(3, 2);
            Figures[k, 0, 3] = new Vector2(4, 2);

            Figures[k, 1, 0] = new Vector2(2, 1);
            Figures[k, 1, 1] = new Vector2(2, 2);
            Figures[k, 1, 2] = new Vector2(2, 3);
            Figures[k, 1, 3] = new Vector2(2, 4);

            Figures[k, 2, 0] = new Vector2(0, 2);
            Figures[k, 2, 1] = new Vector2(1, 2);
            Figures[k, 2, 2] = new Vector2(2, 2);
            Figures[k, 2, 3] = new Vector2(3, 2);

            Figures[k, 3, 0] = new Vector2(2, 0);
            Figures[k, 3, 1] = new Vector2(2, 1);
            Figures[k, 3, 2] = new Vector2(2, 2);
            Figures[k, 3, 3] = new Vector2(2, 3);
            
            k++;

            // J-figures
            Figures[k, 0, 0] = new Vector2(0, 0);
            Figures[k, 0, 1] = new Vector2(0, 1);
            Figures[k, 0, 2] = new Vector2(1, 1);
            Figures[k, 0, 3] = new Vector2(2, 1);

            Figures[k, 1, 0] = new Vector2(1, 0);
            Figures[k, 1, 1] = new Vector2(2, 0);
            Figures[k, 1, 2] = new Vector2(1, 1);
            Figures[k, 1, 3] = new Vector2(1, 2);

            Figures[k, 2, 0] = new Vector2(0, 1);
            Figures[k, 2, 1] = new Vector2(1, 1);
            Figures[k, 2, 2] = new Vector2(2, 1);
            Figures[k, 2, 3] = new Vector2(2, 2);

            Figures[k, 3, 0] = new Vector2(1, 0);
            Figures[k, 3, 1] = new Vector2(1, 1);
            Figures[k, 3, 2] = new Vector2(1, 2);
            Figures[k, 3, 3] = new Vector2(0, 2);

            k++;

            // L-figures
            Figures[k, 0, 0] = new Vector2(0, 1);
            Figures[k, 0, 1] = new Vector2(1, 1);
            Figures[k, 0, 2] = new Vector2(2, 1);
            Figures[k, 0, 3] = new Vector2(2, 0);

            Figures[k, 1, 0] = new Vector2(1, 0);
            Figures[k, 1, 1] = new Vector2(1, 1);
            Figures[k, 1, 2] = new Vector2(1, 2);
            Figures[k, 1, 3] = new Vector2(2, 2);

            Figures[k, 2, 0] = new Vector2(0, 1);
            Figures[k, 2, 1] = new Vector2(1, 1);
            Figures[k, 2, 2] = new Vector2(2, 1);
            Figures[k, 2, 3] = new Vector2(0, 2);

            Figures[k, 3, 0] = new Vector2(0, 0);
            Figures[k, 3, 1] = new Vector2(1, 0);
            Figures[k, 3, 2] = new Vector2(1, 1);
            Figures[k, 3, 3] = new Vector2(1, 2);

            k++;

            // O-figure
            Figures[k, 0, 0] = new Vector2(1, 0);
            Figures[k, 0, 1] = new Vector2(2, 0);
            Figures[k, 0, 2] = new Vector2(1, 1);
            Figures[k, 0, 3] = new Vector2(2, 1);

            Figures[k, 1, 0] = new Vector2(1, 1);
            Figures[k, 1, 1] = new Vector2(1, 2);
            Figures[k, 1, 2] = new Vector2(2, 1);
            Figures[k, 1, 3] = new Vector2(2, 2);

            Figures[k, 2, 0] = new Vector2(0, 1);
            Figures[k, 2, 1] = new Vector2(0, 2);
            Figures[k, 2, 2] = new Vector2(1, 1);
            Figures[k, 2, 3] = new Vector2(1, 2);

            Figures[k, 3, 0] = new Vector2(0, 0);
            Figures[k, 3, 1] = new Vector2(1, 0);
            Figures[k, 3, 2] = new Vector2(0, 1);
            Figures[k, 3, 3] = new Vector2(1, 1);

            k++;

            // S-figures
            Figures[k, 0, 0] = new Vector2(0, 1);
            Figures[k, 0, 1] = new Vector2(1, 1);
            Figures[k, 0, 2] = new Vector2(1, 0);
            Figures[k, 0, 3] = new Vector2(2, 0);

            Figures[k, 1, 0] = new Vector2(1, 0);
            Figures[k, 1, 1] = new Vector2(1, 1);
            Figures[k, 1, 2] = new Vector2(2, 1);
            Figures[k, 1, 3] = new Vector2(2, 2);

            Figures[k, 2, 0] = new Vector2(0, 2);
            Figures[k, 2, 1] = new Vector2(1, 1);
            Figures[k, 2, 2] = new Vector2(1, 2);
            Figures[k, 2, 3] = new Vector2(2, 1);

            Figures[k, 3, 0] = new Vector2(0, 0);
            Figures[k, 3, 1] = new Vector2(0, 1);
            Figures[k, 3, 2] = new Vector2(1, 1);
            Figures[k, 3, 3] = new Vector2(1, 2);

            k++;

            // T-figures
            Figures[k, 0, 0] = new Vector2(0, 1);
            Figures[k, 0, 1] = new Vector2(1, 1);
            Figures[k, 0, 2] = new Vector2(2, 1);
            Figures[k, 0, 3] = new Vector2(1, 0);

            Figures[k, 1, 0] = new Vector2(1, 0);
            Figures[k, 1, 1] = new Vector2(1, 1);
            Figures[k, 1, 2] = new Vector2(1, 2);
            Figures[k, 1, 3] = new Vector2(2, 1);

            Figures[k, 2, 0] = new Vector2(0, 1);
            Figures[k, 2, 1] = new Vector2(1, 1);
            Figures[k, 2, 2] = new Vector2(2, 1);
            Figures[k, 2, 3] = new Vector2(1, 2);

            Figures[k, 3, 0] = new Vector2(1, 0);
            Figures[k, 3, 1] = new Vector2(1, 1);
            Figures[k, 3, 2] = new Vector2(1, 2);
            Figures[k, 3, 3] = new Vector2(0, 1);

            k++;

            // Z-figures
            Figures[k, 0, 0] = new Vector2(0, 0);
            Figures[k, 0, 1] = new Vector2(1, 0);
            Figures[k, 0, 2] = new Vector2(1, 1);
            Figures[k, 0, 3] = new Vector2(2, 1);

            Figures[k, 1, 0] = new Vector2(1, 1);
            Figures[k, 1, 1] = new Vector2(1, 2);
            Figures[k, 1, 2] = new Vector2(2, 0);
            Figures[k, 1, 3] = new Vector2(2, 1);

            Figures[k, 2, 0] = new Vector2(0, 1);
            Figures[k, 2, 1] = new Vector2(1, 1);
            Figures[k, 2, 2] = new Vector2(1, 2);
            Figures[k, 2, 3] = new Vector2(2, 2);

            Figures[k, 3, 0] = new Vector2(0, 1);
            Figures[k, 3, 1] = new Vector2(0, 2);
            Figures[k, 3, 2] = new Vector2(1, 0);
            Figures[k, 3, 3] = new Vector2(1, 1);

            k++;

            #endregion
            return Figures;
        }

        static public void CreateSRSTrueRotationsOffsetData(out Vector2[,] jlstz, out Vector2[,] i, out Vector2[,] o)
        {
            jlstz = new Vector2[4, 5];
            i = new Vector2[4, 5];
            o = new Vector2[4, 5];

            //J, L, S, T, Z
            jlstz[0, 0] = new Vector2(0, 0);
            jlstz[0, 1] = new Vector2(0, 0);
            jlstz[0, 2] = new Vector2(0, 0);
            jlstz[0, 3] = new Vector2(0, 0);
            jlstz[0, 4] = new Vector2(0, 0);

            jlstz[1, 0] = new Vector2(0, 0);
            jlstz[1, 1] = new Vector2(1, 0);
            jlstz[1, 2] = new Vector2(1, 1);
            jlstz[1, 3] = new Vector2(0, -2);
            jlstz[1, 4] = new Vector2(1, -2);

            jlstz[2, 0] = new Vector2(0, 0);
            jlstz[2, 1] = new Vector2(0, 0);
            jlstz[2, 2] = new Vector2(0, 0);
            jlstz[2, 3] = new Vector2(0, 0);
            jlstz[2, 4] = new Vector2(0, 0);

            jlstz[3, 0] = new Vector2(0, 0);
            jlstz[3, 1] = new Vector2(-1, 0);
            jlstz[3, 2] = new Vector2(-1, 1);
            jlstz[3, 3] = new Vector2(0, -2);
            jlstz[3, 4] = new Vector2(-1, -2);
            
            //I
            i[0, 0] = new Vector2(0, 0);
            i[0, 1] = new Vector2(-1, 0);
            i[0, 2] = new Vector2(2, 0);
            i[0, 3] = new Vector2(-1, 0);
            i[0, 4] = new Vector2(2, 0);

            i[1, 0] = new Vector2(-1, 0);
            i[1, 1] = new Vector2(0, 0);
            i[1, 2] = new Vector2(0, 0);
            i[1, 3] = new Vector2(0, -1);
            i[1, 4] = new Vector2(0, 2);

            i[2, 0] = new Vector2(-1, -1);
            i[2, 1] = new Vector2(1, -1);
            i[2, 2] = new Vector2(-2, -1);
            i[2, 3] = new Vector2(1, 0);
            i[2, 4] = new Vector2(-2, 0);

            i[3, 0] = new Vector2(0, -1);
            i[3, 1] = new Vector2(0, -1);
            i[3, 2] = new Vector2(0, -1);
            i[3, 3] = new Vector2(0, 1);
            i[3, 4] = new Vector2(0, -2);

            //O
            o[0, 0] = new Vector2(0, 0);
            o[1, 0] = new Vector2(0, 1);
            o[2, 0] = new Vector2(-1, 1);
            o[3, 0] = new Vector2(-1, 0);

        }
    }
}
