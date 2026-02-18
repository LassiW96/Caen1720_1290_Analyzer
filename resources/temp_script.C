
void temp_analysis() {
   // Example Script to plot Peak Value of Channel 0
   // The GUI automatically opens the selected ROOT file for you.

   TTree *t = (TTree*)gDirectory->Get("t1");
   if (!t) {
       std::cout << "Tree 't1' not found. Make sure you selected a file." << std::endl;
       return;
   }

   // Pre-create the histogram so it is always accessible after Draw
   TH1F *h = new TH1F("h_peak", "Channel 0 Peak Values;ADC Value;Counts", 100, 0, 3000);

   TCanvas *c = new TCanvas("c_ex", "Channel 0 Peak", 800, 600);
   c->cd();

   // Draw peak value of channel 0 into the pre-created histogram
   t->Draw("ch0_peak >> h_peak");

   h->SetTitle("Channel 0 Peak Values;ADC Value;Counts");
   h->Draw();

   c->Draw();
   c->Update();
}
