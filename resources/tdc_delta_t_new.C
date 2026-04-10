void tdc_delta_t() {
  // TDC Delta-T Histogram Script
  // Draws delta-t distributions for any number of channels dynamically.

  TTree *t = (TTree *)gDirectory->Get("tdc");
  if (!t) {
    std::cout << "Tree 'tdc' not found. Make sure you selected a TDC ROOT file."
              << std::endl;
    return;
  }

  TCanvas *c = new TCanvas("c_tdc", "TDC Delta-T Distributions", 900, 600);
  c->cd();

  // 1. Define which channels you want to plot
  std::vector<int> channels_to_plot = {
      1, 2}; // e.g., change to {1, 2, 3, 4, 5} for more
  std::vector<int> colors = {kBlue,   kRed,  kGreen + 2, kMagenta,
                             kOrange, kCyan, kBlack};

  // 2. Setup a THStack to handle automatic Y-axis scaling for multiple overlays
  THStack *hs =
      new THStack("hs", "TDC #Delta t Distributions;#Delta t (ns);Counts");
  TLegend *leg = new TLegend(0.65, 0.45, 0.88, 0.6);
  leg->SetBorderSize(0); // Cleaner look

  // 3. Loop through the channels and fill histograms
  for (size_t i = 0; i < channels_to_plot.size(); ++i) {
    int ch = channels_to_plot[i];
    int color = colors[i % colors.size()]; // Wrap around if you have more
                                           // channels than colors

    TString hName = TString::Format("h_dt0%d", ch);

    // Create the histogram explicitly
    TH1F *h = new TH1F(hName, hName, 100, 0, 40);
    h->SetLineColor(color);
    h->SetLineWidth(2);

    // Formulate the draw and cut commands
    TString drawCmd = TString::Format("delta_t >> %s", hName.Data());

    // --> IMPORTANT: This cut string assumes Option 1 (Vectors)
    TString cutCmd = TString::Format("hit_channels == %d", ch);

    // Draw with "goff" (Graphics OFF) to fill the histogram silently
    t->Draw(drawCmd, cutCmd, "goff");

    // Add to stack and legend
    hs->Add(h);
    leg->AddEntry(h, TString::Format("#Delta t (ch0 - ch%d)", ch), "l");
  }

  // 4. Draw the stack with "nostack" so they overlay instead of adding together
  hs->Draw("nostack hist");
  leg->Draw();

  c->Update();
}