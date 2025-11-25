using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Lab2
{
    internal class Program
    {
        abstract class Organization
        {
            protected Organization(int w, int aP) { workersAmount = w; averageProductivity = aP; }

            public int workersAmount;
            public int averageProductivity;

            public virtual void Data()
            {
                Console.WriteLine($"Name of Organization - Undefined, Count of workers - Undefined");
            }
            public virtual void OverallProductivity()
            {
                Console.WriteLine($"Overall productivity is equal - Undefined");
            }
        }

        abstract class Factory
        {
            protected Factory(int p, int pCnt) { productionByMachine = p; productionCnt = pCnt; }

            protected int productionByMachine;
            protected int productionCnt;
            
            public virtual void MachinesWork()
            {
                Console.WriteLine($"Undefined - number of working machines");
            }
            public virtual void ProducionSummary()
            {
                Console.WriteLine("Num of production is Undefined");
            }
        }

        class InsuranceCompany: Organization
        {
            public string nameOfOrganization;
            public InsuranceCompany(string name, int workersAm, int avProduction) : base(workersAm, avProduction) { }

            public override void Data()  
            {
                Console.WriteLine($"Name of Organization - {0}, Count of workers - {1}", nameOfOrganization, workersAmount);
            }
            public override void OverallProductivity()
            {
                Console.WriteLine($"Overall productivity is equal - {workersAmount * averageProductivity}");
            }
        }

        class OilCompany: Factory
        {
            public OilCompany(int p, int pCnt) : base(p, pCnt) { }

            public override void MachinesWork()
            {
                Console.WriteLine($"{productionCnt / productionByMachine} - number of working machines");
            }
            public override void ProducionSummary()
            {
                Console.WriteLine($"Num of production is {productionCnt}");
            }
        }

        static void Main(string[] args)
        {
        }
    }
}
