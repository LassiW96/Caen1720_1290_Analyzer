
void tdc_delta_t() {
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
  t->Draw("delta_t_ch0_ch1 >> h_dt01(100, 0, 40)", "", "");
  TH1F *h1 = (TH1F *)gDirectory->Get("h_dt01");
  if (h1) {
    h1->SetTitle("TDC #Delta t Distributions;#Delta t (ns);Counts");
    h1->SetLineColor(kBlue);
    h1->SetLineWidth(2);
    h1->Draw();
  }

  // Draw ch0 - ch2 delta-t on same canvas
  t->Draw("delta_t_ch0_ch2 >> h_dt02(100, 0, 40)", "", "same");
  TH1F *h2 = (TH1F *)gDirectory->Get("h_dt02");
  if (h2) {
    h2->SetLineColor(kRed);
    h2->SetLineWidth(2);
    h2->Draw("same");
  }

  // Legend
  TLegend *leg = new TLegend(0.65, 0.45, 0.88, 0.6);
  if (h1)
    leg->AddEntry(h1, "#Delta t (ch0 - ch1)", "l");
  if (h2)
    leg->AddEntry(h2, "#Delta t (ch0 - ch2)", "l");
  leg->Draw();

  c->Update();
}
