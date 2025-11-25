using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace UniFile
{
    class Organization
    {
        private int cntOfPeople;
        private int income;
        private bool license;
        protected void GetDataO()
        {
            Console.WriteLine($"Organization\nCount of people:{this.cntOfPeople}, Income:{this.income}, License:{this.license}\n");
        }
        public Organization() 
        {
            this.cntOfPeople = 0;
            this.income = 0;
            this.license = false;
        }
        public Organization(int cntP, int inc, bool lic) 
        {
            this.cntOfPeople = cntP;
            this.income = inc;
            this.license = lic;
        }
    }

    class Factory: Organization
    {
        private int numOfMachines;
        private int workPerUnit;

        public Factory() : base()
        {
            this.numOfMachines = Convert.ToInt32(Console.ReadLine());
            this.workPerUnit = Convert.ToInt32(Console.ReadLine());
        }
        public Factory(int num, int work) : base()
        {
            this.numOfMachines = num;
            this.workPerUnit = work;
        }
        public Factory(int num, int work, int cntPeople, int income, bool license) : base(cntPeople, income, license)
        {
            this.numOfMachines = num;
            this.workPerUnit = work;
        }

        protected void GetDataF()
        {
            Console.WriteLine($"Factory\nCount of machines{numOfMachines}, Work per unit{workPerUnit}\n");
            GetDataO();
        }
        
    }

    class OilCompany: Factory 
    {
        public int numOfGaloonOfOil;
        public int numOfTradePartners;

        public OilCompany() : base()
        {
            this.numOfGaloonOfOil = Convert.ToInt32(Console.ReadLine());
            this.numOfTradePartners = Convert.ToInt32(Console.ReadLine());
        }
        public OilCompany(int numOfGaloonOfOil, int numOfTradePartners, int num, int work) : base(num, work)
        {
            this.numOfGaloonOfOil = numOfGaloonOfOil;
            this.numOfTradePartners = numOfTradePartners;
        }
        public OilCompany(int numOfGaloonOfOil, int numOfTradePartners, int num, int work, int cntPeople, int income, bool license) : base(num, work, cntPeople, income, license)
        {
            this.numOfGaloonOfOil = numOfGaloonOfOil;
            this.numOfTradePartners = numOfTradePartners;
        }

        public void GetDataOil()
        {
            Console.WriteLine($"OilCompany\nCount of galoon of oil:{numOfGaloonOfOil}, Count of trade partners:{numOfTradePartners}\n");
            GetDataF();
        }
    }

    class InsuranceCompany: Organization 
    {
        public int numOfCustomers;
        public int numOfTradeRequest;
        public InsuranceCompany(): base()
        {
            this.numOfCustomers = Convert.ToInt32(Console.ReadLine());
            this.numOfTradeRequest = Convert.ToInt32(Console.ReadLine());
        }
        public InsuranceCompany(int numOfCust, int numOfTradReq): base()
        {
            this.numOfCustomers = numOfCust;
            this.numOfTradeRequest = numOfTradReq;
        }
        public InsuranceCompany(int numOfCust, int numOfTradReq, int cntP, int inc, bool lic) : base(cntP, inc, lic)
        {
            this.numOfCustomers = numOfCust;
            this.numOfTradeRequest = numOfTradReq;
        }

        public void GetDataIns()
        {
            Console.WriteLine($"Insurance Company\nCount of customers:{numOfCustomers}, Count of trade requests:{numOfTradeRequest}\n");
        }
    }
    

    internal class Program
    {
        static void Main(string[] args)
        {
            InsuranceCompany ins_company_1 = new InsuranceCompany();
            Console.WriteLine();
            InsuranceCompany ins_comp_2 = new InsuranceCompany(Convert.ToInt32(Console.ReadLine()), Convert.ToInt32(Console.ReadLine()));
            ins_comp_2.GetDataIns();
            Console.WriteLine();
            OilCompany oil_factory = new OilCompany();
            oil_factory.GetDataOil();
        }
    }
}
