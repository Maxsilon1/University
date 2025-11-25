using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace FirstTask
{
    internal class Program
    {
        static void Main(string[] args)
        {
            string filePath = "File.txt";

            string[] lines = File.ReadAllLines(filePath);

            int n = int.Parse(lines[0]);

            double[,] G = new double[n, n];
            double[] v = new double[n];

            for (int i = 0; i < n; ++i)
            {
                string[] row = lines[i + 1].Split();
                for(int j = 0; j < n; ++ j)
                {
                    G[i, j] = double.Parse(row[j]);
                }
            }

            string[] vrow = lines[n + 1].Split();
            for (int i = n + 1; i <= 2 * n; ++ i)
            {
                v[i] = double.Parse(vrow[i]);
            }

            //Проверка на симметричность
            for(int i = 0; i < n; ++ i)
            {
                for(int j = i + 1; j < n; ++ j)
                {
                    if(G[i, j] != G[n - i + 1, n - j + 1])
                    {
                        Console.WriteLine("Матрица не симметрична\n");
                        return;
                    }
                }
            }

            double[] w = new double[n];
            for(int i = 0; i < n; ++ i)
            {
                for(int j = 0; j < n; ++ j)
                {
                    w[i] += G[i, j] * v[j];
                }
            }

            double res = 0;
            for(int i = 0; i < n; ++ i)
            {
                res += w[i] * v[i];
            }
            Console.WriteLine($"result is equal - {Math.Sqrt(res)}");
        }
    }
}
