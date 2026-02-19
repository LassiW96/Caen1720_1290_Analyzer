void divided_canvas() {
   // Example Script to plot Peak Values of Channel 0 and Channel 1
   // The GUI automatically opens the selected ROOT file for you.

   TTree *t = (TTree*)gDirectory->Get("t1");
   if (!t) {
       std::cout << "Tree 't1' not found. Make sure you selected a file." << std::endl;
       return;
   }

   // Pre-create histograms
   TH1F *h_ch0 = new TH1F("h_ch0", "Channel 0 Peak;ADC Value;Counts", 100, 0, 3000);
   TH1F *h_ch1 = new TH1F("h_ch1", "Channel 1 Peak;ADC Value;Counts", 100, 0, 3000);

   // Create canvas and divide into 2 pads (2 columns, 1 row)
   TCanvas *c = new TCanvas("c_ex2", "Channel 0 & 1 Peaks", 1000, 500);
   c->Divide(2,1);

   // ---- Pad 1: Channel 0 ----
   c->cd(1);
   t->Draw("ch0_peak >> h_ch0");
   h_ch0->Draw();

   // ---- Pad 2: Channel 1 ----
   c->cd(2);
   t->Draw("ch1_peak >> h_ch1");
   h_ch1->Draw();

   c->Update();
}
