module Spinner = {
  type size = [ | `xs | `sm | `md | `lg];
  type variantColor = [ | `default | `blue | `white];

  [@react.component]
  let make = (~size=`md, ~variantColor=`default) => {
    <div
      className={Cn.make([
        "spinner",
        switch (size) {
        | `xs => "w-6"
        | `sm => "w-14"
        | `md => "w-20"
        | `lg => "w-28"
        },
        switch (variantColor) {
        | `default => "text-gray-800"
        | `blue => "text-blue-500"
        | `white => "text-white"
        },
      ])}
    />;
  };
};

module Button = {
  type variant = [ | `ghost | `contained | `outline];
  type variantColor = [ | `default | `blue];
  type type_ = [ | `button | `submit];

  [@react.component]
  let make =
      (
        ~type_=`button,
        ~children,
        ~disabled=?,
        ~className=?,
        ~variantColor=?,
        ~variant=?,
        ~onClick=?,
        ~isLoading=?,
      ) => {
    let isLoading = isLoading->Option.getWithDefault(false);

    <button
      ?disabled
      type_={
        switch (type_) {
        | `button => "button"
        | `submit => "submit"
        }
      }
      ?onClick
      className={Cn.make([
        "btn flex items-center",
        variantColor->Cn.mapSome(v =>
          switch (v) {
          | `blue => "btn-blue"
          | `default => ""
          }
        ),
        variant->Cn.mapSome(v =>
          switch (v) {
          | `ghost => "btn-ghost"
          | `default => ""
          }
        ),
        className->Cn.unpack,
      ])}>
      {isLoading
         ? <Spinner
             variantColor={
               switch (variantColor) {
               | Some(`blue) => `white
               | Some(`default)
               | _ => `default
               }
             }
             size=`xs
           />
         : React.null}
      <span className={"ml-1"->Cn.ifTrue(isLoading)}> children </span>
    </button>;
  };
};
module IconButton = {
  type variant = [ | `ghost | `contained | `outline];
  type variantColor = [ | `default | `blue];

  [@react.component]
  let make =
      (
        ~type_=?,
        ~icon,
        ~disabled=?,
        ~className=?,
        ~variantColor=?,
        ~onClick=?,
      ) => {
    <button
      ?disabled
      ?type_
      ?onClick
      className={Cn.make([
        "btn flex btn-icon",
        variantColor->Cn.mapSome(v =>
          switch (v) {
          | `blue => "btn-blue"
          | `default => ""
          }
        ),
        className->Cn.unpack,
      ])}>
      icon
    </button>;
  };
};

module Label = {
  [@react.component]
  let make = (~htmlFor, ~children, ~className=?) => {
    <label
      htmlFor
      className={Cn.make([
        "block text-gray-800 text-md font-semibold",
        className->Cn.unpack,
      ])}>
      children
    </label>;
  };
};

module Input = {
  type type_ = [ | `text | `email | `number | `password];

  [@react.component]
  let make =
      (
        ~type_=`text,
        ~onBlur=?,
        ~id=?,
        ~errored=?,
        ~onChange,
        ~value=?,
        ~placeholder=?,
      ) => {
    <input
      type_={
        switch (type_) {
        | `text => "text"
        | `email => "email"
        | `number => "number"
        | `password => "password"
        }
      }
      className={Cn.make([
        "bg-white focus:outline-none focus:shadow-outline border border-gray-300 rounded-md p-2 block w-full appearance-none leading-normal",
        errored->Cn.mapSome(v => v ? "border-red-500" : ""),
      ])}
      ?id
      ?placeholder
      onChange
      ?value
      ?onBlur
    />;
  };
};

module Spacer = {
  [@react.component]
  let make = (~width=?, ~height=?) => {
    <div
      className={Cn.make([
        width->Cn.mapSome(v => "mx-" ++ v->string_of_int),
        height->Cn.mapSome(v => "my-" ++ v->string_of_int),
      ])}
    />;
  };
};

module Divider = {
  [@react.component]
  let make = () => {
    <div className="w-full h-px bg-gray-400" />;
  };
};

[@bs.val]
external addEventListener: (string, 'a => unit) => unit =
  "document.addEventListener";
[@bs.val]
external removeEventListener: (string, 'a => unit) => unit =
  "document.removeEventListener";

type element;

[@bs.send]
external closest: (element, string) => Js.Nullable.t(element) = "closest";

module Dropdown = {
  module Item = {
    [@react.component]
    let make = (~children, ~onClick=?, ~className=?) => {
      <div
        ?onClick
        className={Cn.make([
          "p-2 hover:bg-gray-100 cursor-pointer text-sm flex items-center",
          className->Cn.unpack,
        ])}>
        children
      </div>;
    };
  };
  module Content = {
    [@react.component]
    let make = (~children, ~isVisible) => {
      <div
        className={Cn.make([
          "absolute bottom-auto bg-white right-0 shadow rounded p-2 mt-4 w-48 z-10 ",
          "hidden"->Cn.ifTrue(!isVisible),
        ])}>
        children
      </div>;
    };
  };
  [@react.component]
  let make = (~button, ~elements) => {
    let (isVisible, setDropdownVisibility) = React.useState(() => false);

    React.useEffect0(() => {
      let onClickOutside = event => {
        let dropdownParent = event##target->closest("#dropdown");

        if (Js.Nullable.isNullable(dropdownParent)) {
          setDropdownVisibility(_ => false);
        };
      };

      addEventListener("click", onClickOutside);

      Some(() => removeEventListener("click", onClickOutside));
    });

    <div id="dropdown" className="relative">
      {button(() => setDropdownVisibility(v => !v))}
      <Content isVisible>
        {elements
         ->Array.mapWithIndex((index, element) =>
             <React.Fragment key={"dropdown-item-" ++ string_of_int(index)}>
               element
             </React.Fragment>
           )
         ->React.array}
      </Content>
    </div>;
  };
};

module Alert = {
  type type_ = [ | `success | `error | `info | `warning];
  module Description = {
    [@react.component]
    let make = (~children) => {
      <p className="text-sm font-semibold"> children </p>;
    };
  };

  [@react.component]
  let make = (~children, ~type_=`success) => {
    <div
      className={Cn.make([
        "px-4 py-2 rounded",
        switch (type_) {
        | `success => "bg-green-300"
        | `error => "bg-red-300"
        | `info => "bg-blue-300"
        | `warning => "bg-yellow-300"
        },
      ])}>
      children
    </div>;
  };
};

module Modal = {
  [@react.component]
  let make = (~isVisible, ~onClose, ~children, ~title) => {
    <div
      className={Cn.make([
        "fixed z-30 w-full h-full top-0 left-0",
        "hidden"->Cn.ifTrue(!isVisible),
      ])}>
      <div
        onClick={_ => onClose()}
        className="bg-black opacity-50 fixed top-0 left-0 w-full h-full"
      />
      <div
        className="max-w-screen-md mx-auto mt-10 rounded shadow relative z-40 bg-white p-6">
        <button
          onClick={_ => onClose()}
          className="absolute top-0 mt-4 mr-4 right-0 rounded-full p-2 hover:bg-gray-100 focus:bg-gray-200">
          <BsReactIcons.MdClose size=28 />
        </button>
        <h2 className="text-2xl"> title </h2>
        <div className="mt-4"> children </div>
      </div>
    </div>;
  };
};

module TextField = {
  [@react.component]
  let make =
      (
        ~id,
        ~type_=?,
        ~label,
        ~placeholder=?,
        ~value,
        ~onChange,
        ~error,
        ~onBlur,
      ) => {
    <div className="mb-4">
      <Label htmlFor=id className="mb-2"> label </Label>
      <Input
        ?type_
        ?placeholder
        value
        id
        onBlur
        errored={
          error->Option.mapWithDefault(false, error => error->Result.isError)
        }
        onChange={event => {
          let value = ReactEvent.Form.target(event)##value;

          value->onChange;
        }}
      />
      {switch (error) {
       | Some(Error(message)) =>
         <div className="text-sm text-red-500"> message->React.string </div>
       | Some(Ok(_))
       | None => React.null
       }}
    </div>;
  };
};