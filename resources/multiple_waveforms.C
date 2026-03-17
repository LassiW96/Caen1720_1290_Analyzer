void multiple_waveforms() {
   // Draw multiple waveforms from ch0_samples
   // GUI automatically opens selected ROOT file

   TTree *t = (TTree*)gDirectory->Get("t1");
   if (!t) {
       std::cout << "Tree 't1' not found. Make sure you selected a file." << std::endl;
       return;
   }

   // ===== USER SETTINGS =====
   int startEvent = 0;   // First event to draw
   int nEvents    = 10;  // Number of waveforms to draw
   // =========================

   Long64_t totalEntries = t->GetEntries();

   if (startEvent >= totalEntries) {
       std::cout << "Start event exceeds total entries!" << std::endl;
       return;
   }

   if (startEvent + nEvents > totalEntries) {
       nEvents = totalEntries - startEvent;
   }

   std::vector<int>* samples = nullptr;
   t->SetBranchAddress("ch0_samples", &samples);

   // Create canvas and divide (5x2 layout for 10 plots)
   TCanvas *c = new TCanvas("c_multi_wave", "10 Waveforms", 1400, 800);
   c->Divide(5,2);

   for (int ev = 0; ev < nEvents; ev++) {

       t->GetEntry(startEvent + ev);

       if (!samples) continue;

       int n = samples->size();
       if (n == 0) continue;

       std::vector<double> x(n), y(n);

       for (int i = 0; i < n; i++) {
           x[i] = i;
           y[i] = samples->at(i);
       }

       c->cd(ev + 1);

       TGraph *gr = new TGraph(n, &x[0], &y[0]);
       gr->SetTitle(Form("Event %d;Sample Number;ADC Value",
                         startEvent + ev));
       gr->SetLineWidth(1);
       gr->Draw("AL");
   }

   c->Update();
}
