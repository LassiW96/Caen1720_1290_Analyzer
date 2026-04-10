
void tdc_delta_t_with_stats() {
  // TDC Delta-T Histogram Script
  // Draws ch0-ch1 and ch0-ch2 delta-t distributions on the same canvas.
  // The GUI automatically opens the selected ROOT file for you.

  TTree *t = (TTree *)gDirectory->Get("tdc");
  if (!t) {
    std::cout << "Tree 'tdc' not found. Make sure you selected a TDC ROOT file."
              << std::endl;
    return;
  }

  TCanvas *c = new TCanvas("c_tdc", "TDC Delta-T Distributions", 900, 600);
  c->cd();

  // Draw ch0 - ch1 delta-t
  t->Draw("delta_t_ch0_ch1 >> h_dt01(100, -50, 50)", "", "");
  TH1F *h1 = (TH1F *)gDirectory->Get("h_dt01");
  if (h1) {
    gStyle->SetOptStat(1111);
    h1->SetTitle("TDC #Delta t Distributions;#Delta t (ns);Counts");
    h1->SetLineColor(kBlue);
    h1->SetLineWidth(2);
    // h1->Fit("gaus");
    h1->Draw();
  }

  // Draw ch0 - ch2 delta-t on same canvas
  t->Draw("delta_t_ch0_ch2 >> h_dt02(100, -50, 50)", "", "sames");
  TH1F *h2 = (TH1F *)gDirectory->Get("h_dt02");
  if (h2) {
    gStyle->SetOptStat(1111);
    h2->SetLineColor(kRed);
    h2->SetLineWidth(2);
    // h2->Fit("gaus");
    h2->Draw("sames");
  }

  // Legend
  TLegend *leg = new TLegend(0.75, 0.25, 0.98, 0.4);
  if (h1)
    leg->AddEntry(h1, "#Delta t (ch0 - ch1)", "l");
  if (h2)
    leg->AddEntry(h2, "#Delta t (ch0 - ch2)", "l");
  leg->Draw();

  gPad->Update();

  TPaveStats *st1 = (TPaveStats *)h1->FindObject("stats");
  TPaveStats *st2 = (TPaveStats *)h2->FindObject("stats");

  if (st1 && st2) {
    st2->SetY1NDC(0.6);
    st2->SetY2NDC(0.75);
    st2->SetTextColor(kRed);
    gPad->Modified();
  }

  c->Update();
  // c->SaveAs("~/MyFiles/daqmainFiles/tdcData/WaveGenTest1.png");
}
