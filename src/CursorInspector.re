open Semantics.Core;
open Tyxml_js;
let titlebar = PanelUtils.titlebar;
let typebar_width = 80000;
let html_of_ty = View.html_of_ty(typebar_width);
let typebar = (prefix, ty) => {
  let ty_html = html_of_ty(prefix, ty);
  Html5.(div(~a=[a_class(["infobar", "typebar"])], [ty_html]));
};
let matched_ty_bar = (prefix, ty1, ty2) => {
  let ty1_html = html_of_ty(prefix ++ "-ty1-", ty1);
  let ty2_html = html_of_ty(prefix ++ "-ty2-", ty2);
  Html5.(
    div(
      ~a=[a_class(["infobar", "matched-type-bar"])],
      [
        ty1_html,
        span(~a=[a_class(["matched-connective"])], [pcdata(" ▶ ")]),
        ty2_html,
      ],
    )
  );
};

let special_msg_bar = (msg: string) =>
  Html5.(
    div(~a=[a_class(["infobar", "special-msg-bar"])], [pcdata(msg)])
  );

let expected_indicator = (title_text, type_div) =>
  Html5.(
    div(
      ~a=[a_class(["indicator", "expected-indicator"])],
      [titlebar(title_text), type_div],
    )
  );

let expected_ty_title = "Expecting an expression of type";
let expected_ty_indicator = ty =>
  expected_indicator(expected_ty_title, typebar("expected", ty));
let expected_msg_indicator = msg =>
  expected_indicator("Expecting an expression of ", special_msg_bar(msg));
let expected_any_indicator = expected_msg_indicator("any type");
let expected_a_type_indicator =
  expected_indicator("Expecting ", special_msg_bar("a type"));
let got_indicator = (title_text, type_div) =>
  Html5.(
    div(
      ~a=[a_class(["indicator", "got-indicator"])],
      [titlebar(title_text), type_div],
    )
  );

let got_ty_indicator = ty => got_indicator("Got type", typebar("got", ty));
let got_as_expected_ty_indicator = ty =>
  got_indicator("Got as expected", typebar("got", ty));
let got_inconsistent_indicator = got_ty =>
  got_indicator("Got inconsistent type", typebar("got", got_ty));
let got_inconsistent_matched_indicator = (got_ty, matched_ty) =>
  got_indicator(
    "Got inconsistent type ▶ assumed ",
    matched_ty_bar("got", got_ty, matched_ty),
  );

let got_free_indicator =
  got_indicator("Got a free variable", typebar("got", HTyp.Hole));

let got_consistent_indicator = got_ty =>
  got_indicator("Got consistent type", typebar("got", got_ty));
let got_a_type_indicator = got_indicator("Got", special_msg_bar("a type"));
type err_state_b =
  | TypeInconsistency
  | BindingError
  | OK;
let of_cursor_mode = (cursor_mode: ZExp.cursor_mode) => {
  let (ind1, ind2, err_state_b) =
    switch (cursor_mode) {
    | ZExp.AnaOnly(ty) =>
      let ind1 = expected_ty_indicator(ty);
      let ind2 = got_indicator("Check", special_msg_bar("successful"));
      (ind1, ind2, OK);
    | ZExp.TypeInconsistent(expected_ty, got_ty) =>
      let ind1 = expected_ty_indicator(expected_ty);
      let ind2 = got_inconsistent_indicator(got_ty);
      (ind1, ind2, TypeInconsistency);
    | ZExp.AnaFree(expected_ty) =>
      let ind1 = expected_ty_indicator(expected_ty);
      let ind2 = got_free_indicator;
      (ind1, ind2, BindingError);
    | ZExp.Subsumed(expected_ty, got_ty) =>
      let ind1 = expected_ty_indicator(expected_ty);
      let ind2 =
        HTyp.eq(expected_ty, got_ty) ?
          got_as_expected_ty_indicator(got_ty) :
          got_consistent_indicator(got_ty);
      (ind1, ind2, OK);
    | ZExp.SynOnly(ty) =>
      let ind1 = expected_any_indicator;
      let ind2 = got_ty_indicator(ty);
      (ind1, ind2, OK);
    | ZExp.SynFree =>
      let ind1 = expected_any_indicator;
      let ind2 = got_free_indicator;
      (ind1, ind2, BindingError);
    | ZExp.SynErrorArrow(expected_ty, got_ty) =>
      let ind1 = expected_msg_indicator("function type");
      let ind2 = got_inconsistent_matched_indicator(got_ty, expected_ty);
      (ind1, ind2, TypeInconsistency);
    | ZExp.SynErrorSum(expected_ty, got_ty) =>
      let ind1 = expected_msg_indicator("sum type");
      let ind2 = got_inconsistent_matched_indicator(got_ty, expected_ty);
      (ind1, ind2, TypeInconsistency);
    | ZExp.SynMatchingArrow(syn_ty, matched_ty) =>
      let ind1 = expected_msg_indicator("function type");
      let ind2 =
        switch (syn_ty) {
        | HTyp.Hole =>
          got_indicator(
            "Got type ▶ matched to",
            matched_ty_bar("got", syn_ty, matched_ty),
          )
        | _ => got_indicator("Got", typebar("got", syn_ty))
        };
      (ind1, ind2, OK);
    | ZExp.SynFreeArrow(matched_ty) =>
      let ind1 = expected_msg_indicator("function type");
      let ind2 =
        got_indicator(
          "Got a free variable ▶ matched to",
          matched_ty_bar("got", HTyp.Hole, matched_ty),
        );
      (ind1, ind2, BindingError);
    | ZExp.SynMatchingSum(syn_ty, matched_ty) =>
      let ind1 = expected_msg_indicator("sum type");
      let ind2 =
        switch (syn_ty) {
        | HTyp.Hole =>
          got_indicator(
            "Got type ▶ matched to",
            matched_ty_bar("got", syn_ty, matched_ty),
          )
        | _ => got_indicator("Got", typebar("got", syn_ty))
        };
      (ind1, ind2, OK);
    | ZExp.SynFreeSum(matched_ty) =>
      let ind1 = expected_msg_indicator("sum type");
      let ind2 =
        got_indicator(
          "Got a free variable ▶ matched to",
          matched_ty_bar("got", HTyp.Hole, matched_ty),
        );
      (ind1, ind2, BindingError);
    | ZExp.TypePosition =>
      let ind1 = expected_a_type_indicator;
      let ind2 = got_a_type_indicator;
      (ind1, ind2, OK);
    };

  let cls_of_err_state_b =
    switch (err_state_b) {
    | TypeInconsistency => "cursor-TypeInconsistency"
    | BindingError => "cursor-BindingError"
    | OK => "cursor-OK"
    };

  Html5.(
    div(
      ~a=[a_class(["panel", "cursor-inspector", cls_of_err_state_b])],
      [ind1, ind2],
    )
  );
};

let no_cursor_mode =
  Html5.(
    div(
      ~a=[a_class(["cursor-inspector-body"])],
      [pcdata("Not well typed! This is a bug. Please report.")],
    )
  );

let mk = (cursor_info_rs: Model.cursor_info_rs) => {
  let cursor_inspector_rs =
    React.S.map(
      ({ZExp.mode: cursor_mode, ZExp.form: _, ZExp.ctx: _, _}) => [
        of_cursor_mode(cursor_mode),
      ],
      cursor_info_rs,
    );

  R.Html5.(div(ReactiveData.RList.from_signal(cursor_inspector_rs)));
};
